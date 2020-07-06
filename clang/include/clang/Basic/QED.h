//===--- QED.h - Code QEDation classes --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines classes used for code QEDations such as
//  #pragma clang  qed ...
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_QED_H
#define LLVM_CLANG_BASIC_QED_H

#include "clang/AST/Stmt.h"
#include "llvm/ADT/StringRef.h"

namespace clang {

class QED {
public:
enum Kind {
    UnknownKind,
#define QED_DIRECTIVE(Name) Name##Kind,
#define QED_DIRECTIVE_LAST(Name)                                         \
  QED_DIRECTIVE(Name)                                                    \
  LastKind = Name##Kind
#include "QEDKinds.def"
  };

  static Kind getQEDDirectiveKind(llvm::StringRef Str);
  static llvm::StringRef getQEDDirectiveKeyword(Kind K);

private:
  Kind QEDKind;
  SourceRange LocRange;

protected:
  QED(Kind K, SourceRange LocRange)
      : QEDKind(K), LocRange(LocRange) {}

public:
  virtual ~QED() {}

  Kind getKind() const { return QEDKind; }
  static bool classof(const QED *qed) { return true; }

  /// Source location of the code QEDation directive.
  /// @{
  SourceRange getRange() const { return LocRange; }
  SourceLocation getBeginLoc() const { return LocRange.getBegin(); }
  SourceLocation getEndLoc() const { return LocRange.getEnd(); }
  void setRange(SourceRange L) { LocRange = L; }
  void setRange(SourceLocation BeginLoc, SourceLocation EndLoc) {
    LocRange = SourceRange(BeginLoc, EndLoc);
  }
  /// @}

  /// Each QEDation defines how many loops it consumes and generates.
  /// Users of this class can store arrays holding the information regarding the
  /// loops, such as pointer to the AST node or the loop name. The index in this
  /// array is its "role".
  /// @{
  virtual int getNumInputs() const { return 1; }
  virtual int getNumFollowups() const { return 0; }
  /// @}

  /// A meta role may apply to multiple output loops, its attributes are added
  /// to each of them. A typical example is the 'all' followup which applies to
  /// all loops emitted by a QEDation. The "all" follow-up role is a meta
  /// output whose' attributes are added to all generated loops.
  bool isMetaRole(int R) const { return R == 0; }

  /// Used to warn users that the current LLVM pass pipeline cannot apply
  /// arbitrary QEDation orders yet.
  int getLoopPipelineStage() const;
};

class EddivQED final : public QED {
private:
  int64_t Factor;

  EddivQED(SourceRange Loc, int64_t Factor)
      : QED(EddivKind, Loc), Factor(Factor) {
    assert(Factor >= 2);
  }

public:
  static bool classof(const EddivQED *Eddiv) { return true; }
  static bool classof(const QED *Eddiv) {
    return Eddiv->getKind() == EddivKind;
  }

  /// Create an instance of partial unrolling. The unroll factor must be at
  /// least 2 or -1. When -1, the unroll factor can be chosen by the optimizer.
  /// An unroll factor of 0 or 1 is not valid
  static EddivQED *createFull(SourceRange Loc) {
    EddivQED *Instance = new EddivQED(Loc, -2);
    assert(Instance->isFull());
    return Instance;
  }

  bool isFull() const { return Factor == -2; }

  enum Input { InputToUnroll };
  int getNumInputs() const override { return 1; }

  enum Followup {
    FollowupAll,
    FollowupUnrolled, // only for partial unrolling
    FollowupRemainder // only for partial unrolling
  };
  /*
  int getNumFollowups() const override {
    if (isPartial())
      return 3;
    return 0;
  }*/

  int64_t getFactor() const { return Factor; }
};

} // namespace clang
#endif /* LLVM_CLANG_BASIC_QED_H */