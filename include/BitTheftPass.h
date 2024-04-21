#ifndef LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
#define LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H

#include <llvm/IR/Argument.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <optional>
#include <unordered_map>
#include <vector>

using Element = struct {
  uint64_t size, original_ind;
};
using NewArg = std::vector<Element>;
using Matching = std::vector<NewArg>;

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
    Matching matching(
      std::unordered_map<Argument *, uint64_t> ptrCandidates,
      std::vector<Argument *> intCandidates);
    std::vector<Argument *> getOthers(Function &F, Matching matches);
    void embedAtCaller(CallInst * callInst, Function* caller, Function * callee, Matching matches, std::vector<Argument *> others);
    FunctionType * getEmbeddedFuncTy(Function &F, Matching matches, std::vector<Argument *> others, LLVMContext &C);
    Function * getEmbeddedFunc(Function &F, FunctionType *FTy, StringRef name, Matching matches, std::vector<Argument *> others);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_BIT_THEFT_H
