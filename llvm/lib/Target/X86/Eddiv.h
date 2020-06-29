//===----------------------------Eddiv.h-------------------------------------//
//                                                                           //
//                     The LLVM Compiler Infrastructure                      //
//                                                                           //
// This file is distributed under the Licence of Compiler Tree Technologies  //
//                                                                           //
//===---------------------------------------------------------------------===//
//                                                                           //
//                                                                           //
//===---------------------------------------------------------------------===//

#include "QEDOptions.h"
#include "QEDUtils.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
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

/// Enable or disable this pass from command line
cl::opt<bool> EnableEddiv("enable-eddiv", cl::desc("Call EDDI-V QED Pass"),
                          cl::init(false));

/// Enable or disable this pass from command line
cl::opt<bool> EddivCustomMode("enable-eddiv-custom", cl::desc("Call EDDI-V QED Pass"),
                          cl::init(false));


/// Number of uses of load(Binary opearations) to be cloned
cl::opt<int> EddivDupCount("eddiv-dup-count",
                           cl::desc("Number of binary operations to be "
                                    "duplicated for a given load in eddiv"),
                           cl::init(5));

class Eddiv : public FunctionPass {

private:
  /// Current function
  Function *TheFunction;

  /// Basic Block containing call to cttErr function
  BasicBlock *thenBB;

  /// Error function

  // Constant *errFn;
  FunctionCallee errFn;
  Module *TheModule;

  GlobalVariable *GV;
  GlobalVariable *DupGV;


  //Map to store alloca and duplicate alloca;
  std::map<AllocaInst*, AllocaInst*> AllocaMap;

  //Map to alloca and corrensponding global variable
  std::map<AllocaInst*, GlobalVariable*> AllocaGlobalMap;

  /// Keeps track of all the instructions that are added for QED
  std::vector<Instruction *> QEDInsts;

  /// \brief duplicate the load of alloca
  Instruction *duplicateLoadAndInsert(Instruction *I);

  void handleDuplicateInstruction(Instruction*, Instruction*, AllocaInst*);

  /// \brief Create the thenBB which contains the call to Error Function.
  void createErrorBB();

  /// \brief duplicate the use of a alloca and compare.
  /// \count is depth to which uses has to be duplicated.
  void duplicateInst(AllocaInst &A, int count, Function &F);

  /// \brief duplicate the the alloca instruction
  AllocaInst* duplicateAllocaAndInsert(AllocaInst *AI);

  /// \brief decalred the required functions for this QED transformation
  void declareFunctions(Module *M);

  /// \brief This function adds the metadata and Debug location to all  the
  /// new Instructions.
  void handleNewInst(Instruction *I);

public:
  Eddiv() : FunctionPass(ID) {
    initializeEddivPass(*PassRegistry::getPassRegistry());
  }

  static char ID; // Pass identification, ireplacement for typeid

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override;
};
char Eddiv::ID = 0;
std::map<GlobalVariable*, GlobalVariable*> QEDU::GlobalsMap = {};
}
