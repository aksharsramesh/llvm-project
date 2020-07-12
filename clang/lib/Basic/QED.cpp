//===--- Transform.h - Code transformation classes --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines classes used for code transformations such as
//  #pragma clang transform ...
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/QED.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Casting.h"

using namespace clang;

QED::Kind QED ::getQEDDirectiveKind(llvm::StringRef Str) {
  return llvm::StringSwitch<QED::Kind>(Str)
      .Case("unroll", EddivKind)
      .Default(UnknownKind);
}

llvm::StringRef QED ::getQEDDirectiveKeyword(Kind K) {
  switch (K) {
  case UnknownKind:
    break;
  case EddivKind:
    return "eddiv";
  }
  llvm_unreachable("Not a known transformation");
}

int QED::getLoopPipelineStage() const {
  switch (getKind()) {
  case QED::Kind::EddivKind:
    return cast<EddivQED>(this)->isFull() ? 0 : 4;
  default:
    return -1;
  }
}
