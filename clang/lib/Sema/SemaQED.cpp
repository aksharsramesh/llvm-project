//===---- SemaQED.h ------------------------------------- -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Semantic analysis for code QEDations.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/StmtQED.h"
#include "clang/Basic/QED.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"

using namespace clang;

StmtResult
Sema::ActOnLoopQEDDirective(QED::Kind Kind,
                                  llvm::ArrayRef<QEDClause *> Clauses,
                                  Stmt *AStmt, SourceRange Loc) {
  // TOOD: implement
  return StmtError();
}

QEDClause *Sema::ActOnFullClause(SourceRange Loc) {
  // TOOD: implement
  return nullptr;
}

QEDClause *Sema::ActOnFactorClause(SourceRange Loc, Expr *Factor) {
  // TOOD: implement
  return nullptr;
}
