//===---- ParseTransform.h -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Parse #pragma clang qed ...
//
//===----------------------------------------------------------------------===//

#include "clang/AST/StmtQED.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/RAIIObjectsForParser.h"

using namespace clang;

QED::Kind
Parser::tryParsePragmaQED(SourceLocation BeginLoc,
                                ParsedStmtContext StmtCtx,
                                SmallVectorImpl<QEDClause *> &Clauses) {
  // ... Tok=<transform> | <...> tok::annot_pragma_transform_end ...
  if (Tok.isNot(tok::identifier)) {
    Diag(Tok, diag::err_pragma_qed_expected_directive);
    return QED::UnknownKind;
  }
  std::string DirectiveStr = PP.getSpelling(Tok);
  QED::Kind DirectiveKind =
      QED::getQEDDirectiveKind(DirectiveStr);
  ConsumeToken();

  switch (DirectiveKind) {
  case QED::EddivKind:
    break;
  default:
    Diag(Tok, diag::err_pragma_qed_unknown_directive);
    return QED::UnknownKind;
  }
/*
  while (true) {
    QEDClauseResult Clause = ParseQEDClause(DirectiveKind);
    if (Clause.isInvalid())
      return QED::UnknownKind;
    if (!Clause.isUsable())
      break;

    Clauses.push_back(Clause.get());
  }
*/
  assert(Tok.is(tok::annot_pragma_qed_end));
  return DirectiveKind;
}

StmtResult Parser::ParsePragmaQED(ParsedStmtContext StmtCtx) {
  assert(Tok.is(tok::annot_pragma_qed) && "Not a transform directive!");

  // ... Tok=annot_pragma_transform | <trans> <...> annot_pragma_transform_end
  // ...
  SourceLocation BeginLoc = ConsumeAnnotationToken();

  ParenBraceBracketBalancer BalancerRAIIObj(*this);

  SmallVector<QEDClause *, 8> DirectiveClauses;
  QED::Kind DirectiveKind =
      tryParsePragmaQED(BeginLoc, StmtCtx, DirectiveClauses);
  if (DirectiveKind == QED::UnknownKind) {
    SkipUntil(tok::annot_pragma_qed_end);
    return StmtError();
  }

  assert(Tok.is(tok::annot_pragma_qed_end));
  SourceLocation EndLoc = ConsumeAnnotationToken();

  SourceLocation PreStmtLoc = Tok.getLocation();
  StmtResult AssociatedStmt = ParseStatement();
  if (AssociatedStmt.isInvalid())
    return AssociatedStmt;
  if (!getAssociatedLoop(AssociatedStmt.get()))
    return StmtError(
        Diag(PreStmtLoc, diag::err_pragma_qed_unknown_directive));

  return Actions.ActOnLoopQEDDirective(DirectiveKind, DirectiveClauses,
                                             AssociatedStmt.get(),
                                             {BeginLoc, EndLoc});
}
/*
Parser::QEDClauseResult
Parser::ParseQEDClause(QED::Kind QEDKind) {
  // No more clauses
  if (Tok.is(tok::annot_pragma_qed_end))
    return ClauseEmpty();

  SourceLocation StartLoc = Tok.getLocation();
  if (Tok.isNot(tok::identifier))
    return ClauseError(Diag(Tok, diag::err_pragma_qed_expected_clause));
  std::string ClauseKeyword = PP.getSpelling(Tok);
  ConsumeToken();
  QEDClause::Kind Kind =
      QEDClause::getClauseKind(QEDKind, ClauseKeyword);

  switch (Kind) {
  case QEDClause::UnknownKind:
    return ClauseError(Diag(Tok, diag::err_pragma_qed_unknown_clause));

    // Clauses without arguments.
  case QEDClause::FullKind:
    return Actions.ActOnFullClause(SourceRange{StartLoc, StartLoc});

    // Clauses with integer argument.
  }
  llvm_unreachable("Unhandled clause");
}
*/
