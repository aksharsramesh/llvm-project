//===--- StmtTransform.h - Code transformation AST nodes --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Transformation directive statement and clauses for the AST.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/StmtQED.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtOpenMP.h"

using namespace clang;

bool QEDClause::isValidForQED(QED::Kind QEDKind,
                                          QEDClause::Kind ClauseKind) {
  switch (QEDKind) {
  case clang::QED::EddivKind:
    return ClauseKind == FullKind;
  default:
    return false;
  }
}

QEDClause::Kind
QEDClause ::getClauseKind(QED::Kind QEDKind,
                                llvm::StringRef Str) {
#define QED_CLAUSE(Keyword, Name)                                        \
  if (isValidForQED(QEDKind, QEDClause::Kind::Name##Kind) && \
      Str == #Keyword)                                                         \
    return QEDClause::Kind::Name##Kind;
#include "clang/AST/QEDClauseKinds.def"
  return QEDClause::UnknownKind;
}

llvm::StringRef
QEDClause ::getClauseKeyword(QEDClause::Kind ClauseKind) {
  assert(ClauseKind > UnknownKind);
  assert(ClauseKind <= LastKind);
  static const char *ClauseKeyword[LastKind] = {
#define QED_CLAUSE(Keyword, Name) #Keyword,
#include "clang/AST/QEDClauseKinds.def"

  };
  return ClauseKeyword[ClauseKind - 1];
}

const Stmt *clang::getAssociatedLoop(const Stmt *S) {
  switch (S->getStmtClass()) {
  case Stmt::ForStmtClass:
  case Stmt::WhileStmtClass:
  case Stmt::DoStmtClass:
  case Stmt::CXXForRangeStmtClass:
    return S;
  default:
    return nullptr;
  }
}
