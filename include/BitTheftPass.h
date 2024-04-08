#ifndef LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
#define LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H

#include <llvm/IR/PassManager.h>

namespace llvm {

class BitTheftPass : public PassInfoMixin<BitTheftPass> {
  public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
