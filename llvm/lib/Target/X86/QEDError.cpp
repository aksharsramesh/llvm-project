//===----------------------QEDError.cpp-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the Compiler Tree Technology
// License. See LICENSE.TXT for details.
//
//===------------------------------------------------------------------===//
//
// This file introduces a control flow error
//
//===------------------------------------------------------------------===//

#include "QEDOptions.h"
#include "QEDUtils.h"
#include "X86InstrInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "qed-error"

namespace llvm {

/// Enable or disable this pass from command line
cl::opt<bool> EnableError(
    "enable-error",
    cl::desc("Introduce some control flow error, error will be introduced only "
             "when morethan 3 basic blocks are present in a function"),
    cl::init(false));
} // End of llvm namespace
namespace {
class QEDE : public MachineFunctionPass {
private:
  const TargetInstrInfo *TII;

public:
  QEDE();

  /// Pass identifier.
  static char ID;

  StringRef getPassName() const override { return "QED Error Pass"; }

  bool runOnMachineFunction(MachineFunction &mf);
};

char QEDE::ID = 0;
} // End of anonymus namespace

QEDE::QEDE() : MachineFunctionPass(ID) {}

char &llvm::QEDEID = QEDE::ID;

INITIALIZE_PASS(QEDE, "qed-error", "QEDError Pass", false, false)

/// \brief instroduces a unconditional jump from first block to last block.
/// This is a control flow error. This works only if source has more than three
/// basic blocks and there is no conditional jump from first to last block in
/// the original source.
bool QEDE::runOnMachineFunction(MachineFunction &MF) {

  /// Check if the command line option is specified
  if (!EnableError)
    return false;

  if(QEDU::isQEDLibCall(&MF))
    return false;

  TII = MF.getSubtarget().getInstrInfo();

  MachineBasicBlock &fMBB = MF.front();
  MachineBasicBlock *lMBB = nullptr;
  MachineFunction::iterator MFI = MF.end();
  MachineFunction::iterator MFB = MF.begin();
  DebugLoc DL = fMBB.findDebugLoc(fMBB.end());
  MachineBasicBlock::iterator MBBI = fMBB.end();
  MachineInstr &MI = fMBB.back();
  if(MI.isBranch())
    MI.eraseFromParent();
  unsigned int count = 0;
  for(auto &MBB : MF){
    count++;
    if(count < 3)
      continue;
    auto II = MBB.getFirstNonPHI();
    MachineInstr &MI = *II;
    if(MI.getOpcode()==X86::XOR64ri32){
      lMBB = &MBB;
      BuildMI(fMBB, MBBI, DL, TII->get(X86::JMP_1)).addMBB(&MBB);
      fMBB.addSuccessor(lMBB);
      return true;
    }
  }
  errs()<<"\n Can not introduce error for this test case";
  assert(lMBB && "Can not introduce error for this test case");
  return true;
}

FunctionPass *llvm::createQEDEPass() { return new QEDE(); }
