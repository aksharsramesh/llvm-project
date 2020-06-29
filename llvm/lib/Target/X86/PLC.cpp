//===---------------------------PLC.cpp----------------------------------===//
//                                                                          //
//                     The LLVM Compiler Infrastructure                     //
//                                                                          //
// This file is distributed under the Compiler Tree Technology              //
// License. See LICENSE.TXT for details.                                    //
//                                                                          //
//===--------------------------------------------------------------------===//
//                                                                          //
//  This file implements the PLC method of soc verification as defined in   //
//  IEEE paper "Effective Post-Silicon Validation of                        //
//  System-on-Chips Using Quick Error Detection"                            //
//                                                                          //
//===--------------------------------------------------------------------===//

/// \brief Information about the PLC algorithm implemented.
/// 1. Finds the functtions called by threads.
/// 2. Emits a call to library function in the beginning.
/// 3. This library function loads the passed global variables GV1 and GV2
///    from memory and compares them.
/// 4. GV1 and GV2 are global variables, which eddiv variables, promoted to 
///    global.
/// 5. The library function does locking and unlocking of shared resources 
///    using mutex. So even if there is no mutex lock and unlock in thread 
///    function, global variables will be locked by the library function.
/// 6. There is no PLC transformation without EDDI-V transformation.
/// 7. There is no PLC transformation for single threaded source code.

#include "PLC.h"
#include "QEDOptions.h"
#include "QEDUtils.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h"
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
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/MachinePassRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "plc"

// char PLC::ID;
char &llvm::PLCID = PLC::ID;


INITIALIZE_PASS(PLC, "plc", "PLC pass", false, false)

ModulePass *llvm::createPLCPass() { return new PLC(); }

/// \brief Inserts the declaration of the library function.
void PLC::declareFunctions(Module &M) {

	LLVMContext &TheContext = M.getContext();
	IRBuilder<> builder(TheContext);

	Type *args2[] = {voidPtrTy, voidPtrTy, builder.getInt32Ty(), builder.getInt32Ty()};

	FunctionType *globalFnTy =
		FunctionType::get(voidPtrTy, ArrayRef<Type *>(args2, 4), false);

	globalFn = M.getOrInsertFunction("__cttCompareGlobal", globalFnTy);
}


/// \brief Returns a constant for a int value.
Value *PLC::getConstant(unsigned int n) {
	Value *v = ConstantInt::get(Type::getInt32Ty(TheModule->getContext()), n);
	return v;
}

/// \brief Emits a call instruction before \p I. This is a call to a library 
/// function __cttCompareGlobal which loads GV1 and GV2 from memory and 
/// comapres the result. Mutex locks the shared variables within the library 
/// function.
void PLC::createCall(Value *Type, Value *Signature, Instruction *I, 
    GlobalVariable *GV1, GlobalVariable *GV2) {

	IRBuilder<> Builder(I);
	Value *arg1 = Builder.CreatePointerCast(GV1, voidPtrTy, "arg1");
	Value *arg2 = Builder.CreatePointerCast(GV2, voidPtrTy, "arg2");
	CallInst *CI = Builder.CreateCall(globalFn, {arg1, arg2, Type, Signature});
	QEDU::addMetaData(CI);
	QEDU::addDebugLoc(CI);
	QEDInsts.push_back(CI);
	DEBUG(errs() << "\n Created call for the instruction " << *I);
	DEBUG(errs() << "\n The call instruction is " << *CI);
	return;
}

/// \brief Find the type of argument and decide the arguments to __createThread
/// function.
void PLC::createCallForType(Value *Signature, Instruction *I, Type *Ty, 
    GlobalVariable *GV1, GlobalVariable *GV2) {

	switch (Ty->getTypeID()) {

		case Type::IntegerTyID:
			switch (cast<IntegerType>(Ty)->getBitWidth()) {
				case 1:
				case 16:
					break;
				case 8:
					createCall(getConstant(2), Signature, I, GV1, GV2);
					break;
				default:
					createCall(getConstant(1), Signature, I, GV1, GV2);
					break;
			}
			break;

		case Type::FloatTyID:
			createCall(getConstant(3), Signature, I, GV1, GV2);
			break;

		default:
			DEBUG(errs() << "\n Type not handled" << *Ty);
			break;
	}

	return;
}

/// \brief For two globals \param GV1, \param GV2, this functions performs PLC
void PLC::processGlobal(GlobalVariable *GV1, GlobalVariable *GV2){
  DEBUG(errs()<<"\n The first global is "<<*GV1);
  DEBUG(errs()<<"\n Second global is "<<*GV2);
  BasicBlock *BB = &(ThreadFunction->getEntryBlock());
  Value *signature = getConstant(signatureMap[BB]);
  Instruction *I = &(BB->front());
  DEBUG(errs()<<"\n The first instruction is "<<*I);
  createCallForType(signature, I, GV1->getType()->getElementType(), GV1, GV2);
}

/// \brief For given \param CI this function checks whether this is a call to 
/// thread create and retrieves function called by thread
void PLC::handleCallInst(CallInst* CI){
  Function *F = CI->getCalledFunction();
  if(!F)
    return;
  if(F->getName() != "pthread_create")
    return;
  DEBUG(errs()<<"\n This is a call to pthread create"<<*CI);
  assert(CI->getNumArgOperands() == 4 && "Call to pthread create will have four operands");
  ThreadFunction = dyn_cast<Function>(CI->getArgOperand(2));
  assert(ThreadFunction && "Function can not be null");
  DEBUG(errs()<<"\n The name of function is "<<F->getName());
  for(auto pair : QEDU::GlobalsMap){
    processGlobal(pair.first, pair.second);
  }
}


bool PLC::runOnModule(Module &M) {

	if (!EnablePlc && !PlcCustomMode)
		return false;

  if(QEDU::GlobalsMap.empty()){
    DEBUG(errs()<<"\n NO PLC global variables found");
    return false;
  }

	LLVMContext &TheContext = M.getContext();
	TheModule = &M;

	DEBUG(errs() << "\nPLC Pass for module :" << M.getName());

	voidPtrTy = TypeBuilder<void *, false>::get(TheContext);

	/// Add declarations for the required library functions.
	declareFunctions(M);

	unsigned int count = 0;
	for (Function &F : M) {
		if (QEDU::isQEDLibCall(&F))
			continue;
    if (QEDU::hasNoQEDAttribute(&F)){
      continue;
    }
		Functions.push_back(&F);
		for (BasicBlock &BB : F) {
			count++;
			signatureMap[&BB] = count;
		}
	}
  for(auto F : Functions){
    for(auto &BB : *F){
      for(auto &I : BB){
        CallInst *CI = dyn_cast<CallInst>(&I);
        if(CI)
          CallInsts.push_back(CI);
      }
    }
  }
  for(auto CI : CallInsts){
    handleCallInst(CI);
  }
  if (Verify) {
    for (auto F : Functions)
      QEDU::verify(QEDInsts, *F);
  }

  return true;

}

