/************** QEDMetadataAnalysis.h **************/
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
  struct QEDMetadataAnalysisPass : public FunctionPass {
    static char ID;
    QEDMetadataAnalysisPass();

    bool runOnFunction(Function &F) override;
  }; // end of struct Hello
}  // end of anonymous namespace
