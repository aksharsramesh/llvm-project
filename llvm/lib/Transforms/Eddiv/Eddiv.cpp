//===-----------------------Eddiv.cpp---------------------- ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Compiler Tree Technology
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
//
//
//===---------------------------------------------------------------------===//

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
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "eddiv"

namespace
{
    // Hello - The first implementation, without getAnalysisUsage.
    class Eddiv : public FunctionPass
    {
    public:
        static char ID; // Pass identification, ireplacement for typeid
        BasicBlock *thenBB;
        Eddiv() : FunctionPass(ID) {}
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
        }
        bool runOnFunction(Function &F) override;
        bool isClonable(Instruction *I);
        void duplicateInst(AllocaInst &A, int count, Function &F);
    };
} // namespace

char Eddiv::ID = 0;

//TODO Add the logic to identify all the instructions which are not clonable
//or not need to be cloned

bool Eddiv::isClonable(Instruction *I)
{
    return true;
}

void Eddiv::duplicateInst(AllocaInst &AI, int count, Function &F)
{

    int c = 0;
    SmallVector<Instruction *, 5> instrs;
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for (Use &use : AI.uses())
    {
        c++;
        Instruction *User = cast<Instruction>(use.getUser());
        errs() << "\nUses of alloca found";
        instrs.push_back(User);
        //if(c==count)
        //  break;
    }

    //duplicating the Alloca instruction
    auto newAI = AI.clone();
    BasicBlock::iterator BII(AI);
    ++BII;
    AI.getParent()->getInstList().insert(BII, newAI);

    //duplicating the uses of alloca
    for (Instruction *I : instrs)
    {
        if (I->getOpcode() == Instruction::Store)
            continue;
        if (!isClonable(I))
            continue;
        Instruction *newInst = I->clone();
        BasicBlock::iterator II(I);
        ++II;
        I->getParent()->getInstList().insert(II, newInst);

        //replace all the uses of AI with new AI in newInst
        newInst->replaceUsesOfWith(&AI, newAI);

        // Logic to get the target of original instruction and
        // new instruction and compare them.
        // if both are not equal call error

        Value *value1 = cast<Value>(I);
        Value *value2 = cast<Value>(newInst);

        //Split the basic block
        BasicBlock *BB = I->getParent();
        //Splitting the basic block after cloning the instruction has happened
        BasicBlock *newBB = llvm::SplitBlock(BB, I->getNextNode()->getNextNode(), DT, LI);

        //insert compare instruction before last instruction
        IRBuilder<> Builder(BB->getTerminator());
        Value *condV = Builder.CreateICmpEQ(value1, value2);
        Builder.CreateCondBr(condV, thenBB, newBB);

        //remove the unconditional jump created by SplitBlock
        BB->getTerminator()->eraseFromParent();
    }
}

bool Eddiv::runOnFunction(Function &F)
{

    SmallVector<AllocaInst *, 5> allocas;
    Function *errFunction = F.getParent()->getFunction("cttErr");
    thenBB = BasicBlock::Create(F.getContext(), "then", &F);
    IRBuilder<> Builder(thenBB);
    Builder.CreateCall(errFunction);
    Builder.CreateBr(&F.back());

    if (F.getName() == "cttErr")
        return false;
    for (BasicBlock &BB : F.getBasicBlockList())
    {
        errs() << "\nTraversing the Basic Block";

        //Finding Alloca & Load instructions
        for (auto &Inst : BB)
        {
            auto *AI = dyn_cast<AllocaInst>(&Inst);

            //If the instrution is a alloca instruction
            if (!AI)
            {
                errs() << "\nThis is not a alloca instruction";
                //continue;
            }
            else
            {
                errs() << "\n this is a alloca instruction";
                //duplicateInst(*AI,1);
                allocas.push_back(AI);
            }

            auto LI = dyn_cast<LoadInst>(&Inst);

            //If the instruction is a Load instruction
            //
            // TODO Implement the same logic as that of Alloca
            //
            if (!LI)
            {
                errs() << "\n Load instruction";
            }
            else
                errs() << "\n Not a load instruction";
        }
    }
    for (auto *AI : allocas)
    {
        duplicateInst(*AI, 1, F);
    }
    return true;
}

static RegisterPass<Eddiv> X("eddiv", "Eddiv Pass");