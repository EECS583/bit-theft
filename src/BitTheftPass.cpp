#include "BitTheftPass.h"

#define DEBUG_TYPE "bit-theft"

namespace llvm {

PreservedAnalyses BitTheftPass::run(Function &F, FunctionAnalysisManager &AM) {
    return PreservedAnalyses::all();
}

} // namespace llvm
