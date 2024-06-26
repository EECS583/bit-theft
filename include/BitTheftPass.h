#ifndef LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
#define LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H

#include <llvm/IR/Argument.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <optional>

namespace llvm {

class BitTheftPass : public PassInfoMixin<BitTheftPass> {
  public:
    class Niche {
      protected:
        const Argument *argument = nullptr;
        Align align = Align();

      public:
        Niche(const Argument &argument);

        const Argument *getArgument() const noexcept;
        Align getAlign() const noexcept;
    };

    using BinPack = std::pair<Niche, SmallVector<const Argument *>>;

    static bool isCandidateCalleeFunction(const Function &F);
    static SmallVector<Function *> createCandidateFunctionSnapshot(Module &M);
    static std::optional<Align> getPointerAlign(const DataLayout &L,
                                                const Value &V);
    static SmallVector<BinPack> getBinPackedNiche(const Function &F);
    static std::pair<Function *, SmallVector<unsigned int>>
    createTransformedFunction(
        Function &F, const SmallVector<BitTheftPass::BinPack> &binPacks);

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
