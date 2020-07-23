#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/QEDMetadataAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "example"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct example : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    example() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<QEDMetadataAnalysisPass>();
    }
  };
}

char example::ID = 0;
static RegisterPass<example>
X("example", "example Pass (with getAnalysisUsage implemented)");
