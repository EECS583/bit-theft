#ifndef LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
#define LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H

#include <llvm/IR/Argument.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>
#include <optional>
#include <unordered_map>
#include <vector>

namespace llvm {

class BitTheftPass : public PassInfoMixin<BitTheftPass> {
  public:
    static auto getCandidateCalleeFunctions(Module &M);
    static auto getCandidateCallerFunctions(Module &M);
    static std::optional<Align> getPointerAlignByUser(const Value &V);

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    std::vector<Argument *> getBitTheftCandidate(Function &F);
    std::unordered_map<Argument *, uint64_t>
    getBitTheftCandidatePtr(Function &F);
    uint64_t getMinSpareBitsInPtr(Function &F, Argument *arg);
    std::unordered_map<Argument *, std::vector<Argument *>> matching(
      std::unordered_map<Argument *, uint64_t> ptrCandidates,
      std::vector<Argument *> intCandidates);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
