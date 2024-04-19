#ifndef LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
#define LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H

#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <unordered_map>
#include <vector>

namespace llvm {

class BitTheftPass : public PassInfoMixin<BitTheftPass> {
  public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    std::vector<Argument *> getBitTheftCandidateI1(Function &F);
    std::unordered_map<Argument *, uint64_t>
    getBitTheftCandidatePtr(Function &F);
    uint64_t getMinSpareBitsInPtr(Function &F, Argument *arg);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
