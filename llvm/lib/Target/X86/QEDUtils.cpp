//------------------------------QEDUtils.cpp---------------------------------//
//                                                                           //
//------------------------The LLVM Compiler Infrastructure-------------------//
//                                                                           //
// This file is distributed under the Compiler Tree Technologies             //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
//  This file contains routines that are required for QED.                   //
//                                                                           //
//                                                                           //
//---------------------------------------------------------------------------//

#include "QEDUtils.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/Debug.h"
#include <algorithm>
#include <vector>

#define DEBUG_TYPE "QED"

using namespace llvm;

/// \brief Set get MDString.
static StringRef getMDString() { return "ctt.inserted.for.qed"; }

/// \brief check whether this instruction has a debug location
/// pointing to location 99, 99;
bool QEDU::hasValidDL(Instruction *I) {
  DebugLoc DL = I->getDebugLoc();
  if (!DL)
    return false;
  if (DL.getLine() == 99 && DL.getCol() == 99)
    return true;
  return false;
}

/// \brief Check whether this instruction has a valid Metadata
bool QEDU::hasValidMetadata(Instruction *I) {
  MDNode *MDN = I->getMetadata(getMDString());
  if (!MDN)
    return false;
  return true;
}

/// \brief Check whether this instruction has  valid metadata with a 
/// given string kind.
bool QEDU::hasValidMetadata(Instruction *I, StringRef MDString) {
  MDNode *MDN = I->getMetadata(MDString);
  if (!MDN)
    return false;
  return true;
}

/// \brief Add a unique debug location to the instructions added for QED.
/// This is required to identify these instructions uniquely in MIR.
void QEDU::addDebugLoc(Instruction *I) {
  Function *TheFunction = I->getFunction();
  DebugLoc DL = DebugLoc::get(99, 99, TheFunction->getSubprogram());
  I->setDebugLoc(DL);
  LLVM_DEBUG(errs() << "\n Adding debug location to the instruction " << *I
               << "\n DL is " << DL);
}

/// added for QED add DebugLoc(99,99)
void QEDU::addDebugLoc(MachineInstr *MI) {
  const Function *TheFunction = MI->getParent()->getParent()->getFunction();
  DebugLoc DL = DebugLoc::get(99, 99, TheFunction->getSubprogram());
  MI->setDebugLoc(DL);
}

/// \brief Check whether this is a library call
bool QEDU::isQEDLibCall(Function *F) {
  if (F->getName() == "__cttLoadValue")
    return true;
  if (F->getName() == "__cttCreateThread")
    return true;
  if (F->getName() == "cttErr")
    return true;
  if (F->getName() == "__cttCompareGlobal")
    return true;
  return false;
}

/// \brief Add metadata to the instructions innserted.
void QEDU::addMetaData(Instruction *I) {
  I->setMetadata(getMDString(), MDNode::get(I->getContext(), {}));
}

/// \brief Add metadata to the instructions innserted, 
/// metadata of \param MDString kind.
void QEDU::addMetaData(Instruction *I,StringRef MDString) {
  I->setMetadata(MDString, MDNode::get(I->getContext(), {}));
}

/// \brief Check whether this is a library call
bool QEDU::isQEDLibCall(MachineFunction *MF) {
  if (MF->getName() == "__cttLoadValue")
    return true;
  if (MF->getName() == "__cttCreateThread")
    return true;
  if (MF->getName() == "cttErr")
    return true;
  return false;
}

/// \brief Verifies whether all the instructions added for QED have
/// proper metadata and debug location. This is called only when the
/// user specifies -verify option in command line.
void QEDU::verify(std::vector<Instruction *> QEDInsts, Function &F) {

  LLVM_DEBUG(errs() << "\n Verification for " << F.getName());
  for (BasicBlock &BB : F.getBasicBlockList()) {
    for (auto &I : BB) {
      LLVM_DEBUG(errs() << "\n Checking for I " << I);
      if (std::find(QEDInsts.begin(), QEDInsts.end(), &I) != QEDInsts.end()) {
        LLVM_DEBUG(errs() << "\nQED Instruction ");
        assert(QEDU::hasValidMetadata(&I) &&
               "QED Instruction should have valid QED Metadata");
        assert(QEDU::hasValidDL(&I) &&
               "QED Instruction should have QED debug location");
      } else {
        LLVM_DEBUG(errs() << "\n Normal Instruction");
      }
    }
  }
}


/// \brief Check whether this function has no QED Attribute.
bool QEDU::hasNoQEDAttribute(Function *F){
 return F->hasFnAttribute(Attribute::NoQEDFunction);
}

bool QEDU::hasNoQEDAttribute(MachineFunction *MF){

  Function *F = const_cast<Function*>(MF->getFunction());
  return hasNoQEDAttribute(F);

}
