//===---- ParseQED.h -------------------------------------*- C++ -*-===//
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
  llvm::errs() << "here\n";
  if (Tok.isNot(tok::annot_pragma_qed_end)) {
    Diag(Tok, diag::err_pragma_qed_expected_directive);
    return QED::UnknownKind;
  }
  // ... Tok=<QED> | <...> tok::annot_pragma_QED_end ...
  /*
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

  while (true) {
    QEDClauseResult Clause = ParseQEDClause(DirectiveKind);
    if (Clause.isInvalid())
      return QED::UnknownKind;
    if (!Clause.isUsable())
      break;

    Clauses.push_back(Clause.get());
  }

  assert(Tok.is(tok::annot_pragma_qed_end));
  */
  return QED::EddivKind;
}

StmtResult Parser::ParsePragmaQED(ParsedStmtContext StmtCtx) {
  assert(Tok.is(tok::annot_pragma_qed) && "Not a QED directive!");

  // ... Tok=annot_pragma_QED | <trans> <...> annot_pragma_QED_end
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
  StmtResult AssociatedStmt = ParseQEDStatement();
  llvm::errs()<<"in ParsePragmaQED\n";
  /*
  if (AssociatedStmt.isInvalid())
    return AssociatedStmt;
  if (!getAssociatedLoop(AssociatedStmt.get()))
    return StmtError(
        Diag(PreStmtLoc, diag::err_pragma_qed_expected_clause));

  return Actions.ActOnLoopQEDDirective(DirectiveKind, DirectiveClauses,
                                             AssociatedStmt.get(),
                                             {BeginLoc, EndLoc});
                                             */
  AssociatedStmt.get()->setIsQEDStmt();
  return AssociatedStmt;
}

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
    case QEDClause::FactorKind: {
    BalancedDelimiterTracker T(*this, tok::l_paren,
                               tok::annot_pragma_qed_end);
    if (T.expectAndConsume(diag::err_expected_lparen_after,
                           ClauseKeyword.data()))
      return ClauseError();

    ExprResult Expr = ParseConstantExpression();
    if (Expr.isInvalid())
      return ClauseError();

    if (T.consumeClose())
      return ClauseError();
    SourceLocation EndLoc = T.getCloseLocation();
    SourceRange Range{StartLoc, EndLoc};
    switch (Kind) {
    case QEDClause::FactorKind:
      return Actions.ActOnFactorClause(Range, Expr.get());
    default:
      llvm_unreachable("Unhandled clause");
    }
  }
  }
  llvm_unreachable("Unhandled clause");
}
