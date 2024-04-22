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
    static bool isCandidateCalleeFunction(const Function &F);
    static std::optional<Align> getPointerAlign(const Module &M,
                                                const Value &V);

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

    static SmallVector<BinPack> getBinPackedNiche(const Function &F);
    static bool isNiche(const SmallVector<BinPack> &binPacks, const Value &V);
    static bool isThief(const SmallVector<BinPack> &binPacks, const Value &V);
    static std::pair<Function *, SmallVector<unsigned int>>
    createTransformedFunction(
        Function &F, const SmallVector<BitTheftPass::BinPack> &binPacks);

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
