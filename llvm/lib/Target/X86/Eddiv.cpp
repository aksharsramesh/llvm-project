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

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/Passes.h"
#include "Eddiv.h"

using namespace llvm;

#define DEBUG_TYPE "eddiv"

char &llvm::EddivID = Eddiv::ID;

INITIALIZE_PASS(Eddiv, "eddiv", "Eddiv pass", false, false)

FunctionPass *llvm::createEddivPass()
{
    return new Eddiv();
}

/// Chekck the whether the function is ctt error function
bool Eddiv::isCttErr(Function &F)
{
    if (F.getName() == "cttErr")
        return true;
    return false;
}

/// Add metadat to the instructions innserted
void Eddiv::addMetaData(Instruction *I)
{

    errs() << "\nAdding metadata";
    I->setMetadata(Eddiv::getMDString(), MDNode::get(I->getContext(), {}));
}

/// Add the logic to identify all the instructions which are not clonable or
/// not need to be cloned
bool Eddiv::isClonable(Instruction *I)
{

    //If the instruction is store instruction.
    if (I->getOpcode() == Instruction::Store)
        return false;

    //if the instruction has meta data added for QED
    if (I->getMetadata(Eddiv::getMDString()))
        return false;
    return true;
}

/// Function duplicates the AllocaInstruction(AI) and all its uses.
/// Replaces AI in duplciated uses of AI with duplicate AI.
void Eddiv::duplicateInst(AllocaInst &AI, int count, Function &F)
{

    int c = 0;
    SmallVector<Instruction *, 5> instrs;
    for (Use &use : AI.uses())
    {
        c++;
        Instruction *User = cast<Instruction>(use.getUser());
        LLVM_DEBUG(dbgs() << "\nUses of alloca found");
        instrs.push_back(User);
    }

    ///duplicating the Alloca instruction
    auto newAI = AI.clone();
    addMetaData(newAI);
    BasicBlock::iterator BII(AI);
    ++BII;
    AI.getParent()->getInstList().insert(BII, newAI);

    /// Duplicating the uses of alloca.
    for (Instruction *I : instrs)
    {
        if (!isClonable(I))
            continue;
        Instruction *newInst = I->clone();
        BasicBlock::iterator II(I);
        ++II;
        I->getParent()->getInstList().insert(II, newInst);

        /// Replace all the uses of AI with new AI in newInst.
        newInst->replaceUsesOfWith(&AI, newAI);

        /// Add the metadata to the duplicate instruction
        addMetaData(newInst);
        BasicBlock *BB = I->getParent();

        /// Split the original basic block and add EDDI-V instructions.
        BasicBlock *newBB = BB->splitBasicBlock(I->getNextNode()->getNextNode());

        /// Insert compare instruction before last instruction.
        IRBuilder<> Builder(BB->getTerminator());
        Value *condV = Builder.CreateICmpEQ(I, newInst);
        Builder.CreateCondBr(condV, newBB, thenBB);

        /// Remove the unconditional jump created by SplitBlock.
        BB->getTerminator()->eraseFromParent();

        /// Cloning only specified uses of actual instruction.
        c++;
        if (c == count)
            break;
    }
}

bool Eddiv::runOnFunction(Function &F)
{

    LLVM_DEBUG(dbgs() << "****************Eddiv Pass**************");
    SmallVector<AllocaInst *, 5> allocas;
    Function *errFunction = F.getParent()->getFunction("cttErr");
    thenBB = BasicBlock::Create(F.getContext(), "ctt.qed.then", &F);
    IRBuilder<> Builder(thenBB);
    Builder.CreateCall(errFunction);
    Builder.CreateBr(&F.back());

    ///If the function is ctt error don't do any transformation
    if (isCttErr(F))
        return false;
    for (BasicBlock &BB : F.getBasicBlockList())
    {

        //Finding Alloca & Load instructions
        for (auto &Inst : BB)
        {

            auto *AI = dyn_cast<AllocaInst>(&Inst);
            if (!AI)
            {
                LLVM_DEBUG(dbgs() << "\nThis is not a alloca instruction");
            }
            else
            {
                //If not alloca
                LLVM_DEBUG(dbgs() << "\n this is a alloca instruction");
                //duplicateInst(*AI,1);
                allocas.push_back(AI);
            }
        }
    }
    for (auto *AI : allocas)
    {
        duplicateInst(*AI, 2, F);
    }
    return true;
}
