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
#include "llvm/InitializePasses.h"

namespace llvm
{
    class Eddiv : public FunctionPass
    {

    private:
        static StringRef getMDString()
        {
            return "ctt.inserted.for.qed";
        }

        /// Add metadata. to the instructions innserted
        void addMetaData(Instruction *I);
        /// Add the logic to identify all the instructions which are not clonable
        /// ornot need to be cloned
        bool isClonable(Instruction *I);
        bool isCttErr(Function &F);

    public:
        static char ID; // Pass identification, ireplacement for typeid
        BasicBlock *thenBB;

        Eddiv() : FunctionPass(ID)
        {
            initializeEddivPass(*PassRegistry::getPassRegistry());
        }
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
        }
        bool runOnFunction(Function &F) override;
        void duplicateInst(AllocaInst &A, int count, Function &F);
    };
    char Eddiv::ID = 0;
} // namespace llvm
