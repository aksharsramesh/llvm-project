//===-----------------------Eddiv.cpp---------------------- ---------------===//
//                                                                            //
//                     The LLVM Compiler Infrastructure                       //
//                                                                            //
// This file is distributed under the Compiler Tree Technology                //
// License. See LICENSE.TXT for details.                                      //
//                                                                            //
//===-------------------------------------------------------------------- -===//
//                                                                            //
// IMplementation of EDDI-V method of SoC verification                        //
//                                                                            //
// \brief clones alloca and uses of a alloca and its uses.                    //
// Compares the results.                                                      //
//                                                                            //
//===----------------------------------------------------------------------===//

/// \brief Information about the algorithm used.
/// 1. Find all the Alloca instructions
/// 2. Duplicate the load of alloca instruction
/// 3. For every use load of alloca duplicate it and compare the result with
///    Original instruction.
/// 4. If they are not equal an error is detected.
/// Only uses which have binary operatios are duplicated.
/// No of alloca to use can be controlled from command line.

#include "Eddiv.h"
#include "QEDOptions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/DebugInfoMetadata.h"
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
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "eddiv"

char &llvm::EddivID = Eddiv::ID;


INITIALIZE_PASS(Eddiv, "eddiv", "Eddiv pass", false, false)

FunctionPass *llvm::createEddivPass() { return new Eddiv(); }

/// \brief Check whether this instructuction can be a valid operator for ICmp
static bool isNotValidICmp(Instruction *I) {
  return (!(I->getType()->isIntegerTy() || I->getType()->isPointerTy()));
}

/// \brief Add the logic to identify all the instructions which are not clonable
/// or not need to be cloned
static bool isClonable(Instruction *I) {

  if (QEDU::hasValidMetadata(I))
    return false;
  if (isNotValidICmp(I))
    return false;
  return isa<BinaryOperator>(I);
}

/// \breif Returns Eddiv Metadata String
static StringRef getEddivMDString(){
  return "qed.eddiv.var";
}

/// \brief Checks whether this alloca variable has Eddiv metadata.
static bool isEddivVar(Instruction *I){
  if(QEDU::hasValidMetadata(I, getEddivMDString()))
    return true;
  return false;
}

/// \brief This function adds the metadata and Debug location to all  the
/// new Instructions.
void Eddiv::handleNewInst(Instruction *I) {
  QEDU::addMetaData(I);
  QEDU::addDebugLoc(I);
  QEDInsts.push_back(I);
}

/// \brief duplicate the load of alloca
Instruction *Eddiv::duplicateLoadAndInsert(Instruction *I) {
  auto LI = I->clone();
  BasicBlock::iterator II(I);
  ++II;
  I->getParent()->getInstList().insert(II, LI);
  handleNewInst(LI);
  return LI;
}

/// \brief duplicates the alloca instrucions and inserts back to the function 
AllocaInst* Eddiv::duplicateAllocaAndInsert(AllocaInst *AI){
  if(AllocaMap[AI]){
    GV = AllocaGlobalMap[AI];
    DupGV = AllocaGlobalMap[AllocaMap[AI]];
    return AllocaMap[AI];
  }
  auto newAI = AI->clone();
  BasicBlock::iterator II(AI);
  ++II;
  AI->getParent()->getInstList().insert(II, newAI);
  handleNewInst(newAI);
  AllocaInst *AINew = dyn_cast<AllocaInst>(newAI);
  AllocaMap[AI] = AINew;

  std::string name1 = (TheFunction->getName() + "_" + AI->getName()).str();
  std::string name2 = (TheFunction->getName() + "_" + AI->getName() + "_").str();

  GV = new GlobalVariable(*TheModule, AI->getType()->getElementType(), 
       false, GlobalValue::ExternalLinkage, 0,
        name1);
  DupGV = new GlobalVariable(*TheModule, AI->getType()->getElementType(), 
       false, GlobalValue::ExternalLinkage, 0,
        name2);

  GV->setVisibility(GlobalValue::DefaultVisibility);
  Constant *AggZero = Constant::getNullValue(AI->getType()->getElementType());
  GV->setInitializer(AggZero);
  DupGV->setVisibility(GlobalValue::DefaultVisibility);
  DupGV->setInitializer(AggZero);

  assert(GV && DupGV && "Global Variable can not be null");
 
  AllocaGlobalMap[AI] = GV;
  AllocaGlobalMap[AINew] = DupGV;
  QEDU::GlobalsMap[GV] = DupGV;
  return AINew;
}

/// \breif Finds the uses of instruction, if the use is store instrucion to alloca
// then duplicates such allocas and stores the value of newInst to such allocas
void Eddiv::handleDuplicateInstruction(Instruction *Inst, Instruction *newInst,
    AllocaInst *AII){

  IRBuilder<> Builder(TheModule->getContext());
  for(Use &use : Inst->uses()){
    Instruction *User = dyn_cast<Instruction>(use.getUser());
    if(dyn_cast<StoreInst>(User)){
      LLVM_DEBUG(errs()<<"\n Found the store use : "<<*User);
      Instruction *I = dyn_cast<Instruction>(User->getOperand(1));
      assert(I);
      AllocaInst *AI = dyn_cast<AllocaInst>(I);
      if(AI){
        LLVM_DEBUG(errs()<<"\n Alloca found "<<*I);
        AI = duplicateAllocaAndInsert(AI);
        assert(AI && "New Alloca can not be null");
        assert(GV && "GlobalVariable can not be null");
        assert(DupGV && "Duplicate Global can not be null");
        Builder.SetInsertPoint(User->getNextNode());
        handleNewInst(Builder.CreateStore(newInst, AI));
        handleNewInst(Builder.CreateStore(Inst, GV));
        handleNewInst(Builder.CreateStore(newInst, DupGV));
      }
    }
  }
}




/// \brief Finds all the loads of alloca and duplicates them.
/// For a given pair of original load and duplicated load duplicates
/// count uses of that orinal load which are binary operations.
/// Then replace all the uses of original load with duplicated in
/// duplicate binary operation and then compare the results of them.
void Eddiv::duplicateInst(AllocaInst &AI, int count, Function &F) {

  int c = 0;
  SmallVector<Instruction *, 5> instrs;
  SmallVector<Instruction *, 5> loads;

  // find all the loads of a alloca
  for (Use &use : AI.uses()) {
    c++;
    Instruction *User = cast<Instruction>(use.getUser());
    LLVM_DEBUG(dbgs() << "\nUses of alloca found");
    if (User->getOpcode() == Instruction::Load)
      loads.push_back(User);
  }

  // duplicate all the loads of alloca.
  Instruction *newLI;
  for (auto LI : loads) {

    newLI = duplicateLoadAndInsert(LI);

    /// Duplicating the uses of Load
    /// Keeping track uses duplicated
    c = 0;
    for (Value::use_iterator UI = LI->use_begin(), UE = LI->use_end();
        (UI != UE && c != count); ++UI) {

      Use &use = *UI;
      Instruction *I = cast<Instruction>(use.getUser());
      if (!isClonable(I))
        continue;

      // If this instruction is already duplicated dont duplicate it again
      if(QEDU::hasValidMetadata(I, "qed.used.for.eddiv"))
        continue;
      QEDU::addMetaData(I, "qed.used.for.eddiv");
      Instruction *newInst = I->clone();
      BasicBlock::iterator II(I);
      ++II;
      I->getParent()->getInstList().insert(II, newInst);

      /// Replace all the uses of LI with new LI in newInst.
      newInst->replaceUsesOfWith(LI, newLI);

      /// Add the metadata to the duplicate instruction
      handleNewInst(newInst);
      BasicBlock *BB = I->getParent();

      /// Split the original basic block and add EDDI-V instructions.
      BasicBlock *newBB = BB->splitBasicBlock(I->getNextNode()->getNextNode());

      /// Insert compare instruction before last instruction.
      IRBuilder<> Builder(BB->getTerminator());
      Value *condV = Builder.CreateICmpEQ(I, newInst);
      if (Instruction *CondI = dyn_cast<Instruction>(condV)) {
        handleNewInst(CondI);
      }
      BranchInst *BI = Builder.CreateCondBr(condV, newBB, thenBB);
      handleNewInst(BI);
      /// Remove the unconditional jump created by SplitBlock.
      BB->getTerminator()->eraseFromParent();

      // Keeping track of number of uses duplicated
      c++;
      handleDuplicateInstruction(I, newInst, &AI); 
    }
  }
}

/// \brief Inserts the declaration of the library function
void Eddiv::declareFunctions(Module *M) {
  // Declaration of cttErr
  IRBuilder<> builder(M->getContext());
  FunctionType *cttErrTy = FunctionType::get(builder.getVoidTy(), false);
  errFn = M->getOrInsertFunction("cttErr", cttErrTy);
}

/// \brief Create the thenBB which contains the call to Error Function.
void Eddiv::createErrorBB() {
  thenBB = BasicBlock::Create(TheFunction->getContext(), "ctt.qed.then",
      TheFunction);
  IRBuilder<> Builder(thenBB);
  CallInst *CI = Builder.CreateCall(errFn);
  BranchInst *BI = Builder.CreateBr(&TheFunction->back());
  // Add the metadata and debug information.
  handleNewInst(CI);
  handleNewInst(BI);
}


bool Eddiv::runOnFunction(Function &F) {

  TheFunction = &F;

  if (!EnableEddiv && !EddivCustomMode)
    return false;

  if (QEDU::isQEDLibCall(&F))
    return false;
  if (QEDU::hasNoQEDAttribute(&F)){
    return false;      
  }

  LLVM_DEBUG(dbgs() << "****************Eddiv Pass**************");
  SmallVector<AllocaInst *, 5> allocas;

  TheModule = F.getParent();
  /// Declare Error Function
  declareFunctions(F.getParent());

  /// Create BasicBlock containing call to cttErr
  createErrorBB();

  for (BasicBlock &BB : F.getBasicBlockList()) {

    // Finding Alloca instructions
    for (auto &Inst : BB) {

      auto *AI = dyn_cast<AllocaInst>(&Inst);
      if (!AI) {
        LLVM_DEBUG(dbgs() << "\nThis is not a alloca instruction");
        break;
      }
      // If not alloca
      LLVM_DEBUG(dbgs() << "\n this is a alloca instruction" << *AI);
      // duplicateInst(*AI,1);

      //Selecting allocas with metadata
      if(EddivCustomMode){
        if(isEddivVar(AI))
          allocas.push_back(AI);
        continue;
      }
      allocas.push_back(AI);
      AllocaMap[AI] = nullptr;
    }
  }
  //int c = 0;
  for (auto *AI : allocas) {
    //if (c >= QedDepth)
    //break;
    duplicateInst(*AI, EddivDupCount, F);
    //c++;
  }
  if (Verify)
    QEDU::verify(QEDInsts, F);
  return true;
}
