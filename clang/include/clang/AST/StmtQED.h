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

#ifndef LLVM_CLANG_AST_STMTQED_H
#define LLVM_CLANG_AST_STMTQED_H

#include "clang/AST/Stmt.h"
#include "clang/Basic/QED.h"
#include "llvm/Support/raw_ostream.h"

namespace clang {

/// Represents a clause of a \p QEDExecutableDirective.
class QEDClause {
public:
  enum Kind {
    UnknownKind,
#define QED_CLAUSE(Keyword, Name) Name##Kind,
#define QED_CLAUSE_LAST(Keyword, Name) Name##Kind, LastKind = Name##Kind
#include "clang/AST/QEDClauseKinds.def"
  };

  static bool isValidForQED(QED::Kind QEDKind,
                                  QEDClause::Kind ClauseKind);
  static Kind getClauseKind(QED::Kind QEDKind, llvm::StringRef Str);
  static llvm::StringRef getClauseKeyword(QEDClause::Kind ClauseKind);

  // TODO: implement
};

/// Represents
///
///   #pragma clang qed
///
/// in the AST.
class QEDExecutableDirective final {
  // TODO: implement
};

const Stmt *getAssociatedLoop(const Stmt *S);
} // namespace clang

#endif /* LLVM_CLANG_AST_STMTQED_H */