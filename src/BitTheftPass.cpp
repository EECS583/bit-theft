#include "BitTheftPass.h"

#define DEBUG_TYPE "bit-theft"

namespace llvm {

PreservedAnalyses BitTheftPass::run(Function &F, FunctionAnalysisManager &AM) {
    errs() << F.getName() << '\n';
    for (const auto &argument : F.args()) {
        errs() << argument << '\n';
    }
    return PreservedAnalyses::all();
}

} // namespace llvm
