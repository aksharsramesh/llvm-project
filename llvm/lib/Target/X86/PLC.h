//===----------------------------PLC.h------------------------------------===//
//                                                                           //
//                     The LLVM Compiler Infrastructure                      //
//                                                                           //
// This file is distributed under the Compiler Tree Technology               //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//===---------------------------------------------------------------------===//
//                                                                           //
//  Header for PLC.cpp                                                       //
//                                                                           //
//===---------------------------------------------------------------------===//

#include "QEDOptions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/DerivedTypes.h"

namespace llvm {

cl::opt<bool> EnablePlc("enable-plc", cl::desc("Call PLC Pass"),
                        cl::init(false));

cl::opt<bool> PlcCustomMode(
    "enable-plc-custom",
    cl::desc("PLC custom mode where the user can specify the variables in "
             "source on which PLC will be performed"),
    cl::init(false));

cl::opt<int> QedDepth(
    "qed-depth",
    cl::desc("number of load instructions on which PLC needs to performed"),
    cl::init(5));

// cl::opt<bool> Verify("qed-verify", cl::desc("Verify the QED"), cl::init(false));

class PLC : public ModulePass {

  std::vector<Function *> Functions;
  Module *TheModule;

  /// Keeps track of all the instructions that are added for QED
  std::vector<Instruction *> QEDInsts;

  /// Keeps track of all the call instructions.
  std::vector<CallInst *> CallInsts;

  /// void* type.
  Type *voidPtrTy;


  /// Library function to compare the values of global variables.
  // Constant *globalFn;
  FunctionCallee globalFn;

  /// Function which is called by thread
  Function *ThreadFunction;

  /// Map to hold the signature for each basicblock.
  std::map<BasicBlock *, unsigned int> signatureMap;

  /// \brief Returns a constant for a int value.
  Value *getConstant(unsigned int n);


  /// \brief Declares the required library functions.
  void declareFunctions(Module &M);

  /// \brief Creates the call to __cttCompareGlobal.
  void createCall(Value *Type, Value *Signature, Instruction *I, 
      GlobalVariable *GV1, GlobalVariable *GV2);

  void createCallForType(Value *Signature, Instruction *I, Type *Ty, 
      GlobalVariable *GV1, GlobalVariable *GV2);

  /// \brief Handle the call to pthread_create.
  void handleCallInst(CallInst *CI);

  /// \brief Handle the two global variables and perform PLC operations.
  void processGlobal(GlobalVariable *GV1, GlobalVariable *GV2);


public:
  static char ID; // Pass identification, ireplacement for typeid

  PLC() : ModulePass(ID) {
    initializePLCPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override;
};
char PLC::ID = 0;
} // namespace llvm
