/************** QEDMetadataAnalysisPass.cpp **************/
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/QEDMetadataAnalysis.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

QEDMetadataAnalysisPass::QEDMetadataAnalysisPass() : FunctionPass(ID) {
  initializeQEDMetadataAnalysisPassPass(*PassRegistry::getPassRegistry());
}

bool QEDMetadataAnalysisPass::runOnFunction(Function &F) {
  errs() << "Hello: ";
  errs().write_escaped(F.getName()) << '\n';
  return false;
}

char QEDMetadataAnalysisPass::ID = 0;

INITIALIZE_PASS_BEGIN(QEDMetadataAnalysisPass, "qed-metadata", "QED Metadata Analysis",
		      true, true)
INITIALIZE_PASS_END(QEDMetadataAnalysisPass, "loops", "QED Metadata Analysis",
                    true, true)
