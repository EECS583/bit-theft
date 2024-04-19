#include "BitTheftPass.h"

#define DEBUG_TYPE "bit-theft"

namespace llvm {

PreservedAnalyses BitTheftPass::run(Module &M, ModuleAnalysisManager &AM) {
    errs() << "Module Pass: " << M.getName() << '\n';
    for (auto &function : M.functions()) {
        if (function.getName().startswith("llvm.")) {
            continue;
        }
        errs() << function.getName() << '\n';
    }
    return PreservedAnalyses::all();
}

} // namespace llvm
