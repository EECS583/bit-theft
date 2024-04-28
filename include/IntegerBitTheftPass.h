#ifndef LLVM_TRANSFORMS_SCALAR_INTEGER_BIT_THEFT_H
#define LLVM_TRANSFORMS_SCALAR_INTEGER_BIT_THEFT_H

#include <llvm/IR/Argument.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <optional>

namespace llvm {

class IntegerBitTheftPass : public PassInfoMixin<IntegerBitTheftPass> {
  public:
    static bool isThief(const Value &V);
    static bool isCandidateCalleeFunction(const Function &F);
    static SmallVector<Function *> createCandidateFunctionSnapshot(Module &M);
    static std::pair<Function *, SmallVector<unsigned int>>
    createTransformedFunction(Function &F);

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEGER_BIT_THEFT_H
