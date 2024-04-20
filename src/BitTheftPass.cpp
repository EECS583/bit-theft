#include "BitTheftPass.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#define DEBUG_TYPE "bit-theft"

namespace llvm {

std::vector<Argument *> BitTheftPass::getBitTheftCandidateI1(Function &F) {
    std::vector<Argument *> candidates;
    for (auto &arg : F.args()) {
        if (arg.getType()->isIntegerTy(1)) {
            candidates.push_back(&arg);
        }
    }
    return candidates;
}

std::unordered_map<Argument *, uint64_t>
BitTheftPass::getBitTheftCandidatePtr(Function &F) {
    std::unordered_map<Argument *, uint64_t> candidates;
    for (auto &arg : F.args()) {
        if (arg.getType()->isPointerTy()) {
            candidates[&arg] = getMinSpareBitsInPtr(F, &arg);
        }
    }
    return candidates;
}

uint64_t BitTheftPass::getMinSpareBitsInPtr(Function &F, Argument *arg) {
    size_t minAlignment = std::numeric_limits<size_t>::max();
    for (auto &block : F) {
        for (auto &inst : block) {
            // If the instruction is a load instruction
            // and the pointer operand is the argument
            // we are interested in, we can check the
            // alignment of the load instruction.
            if (auto *loadInst = dyn_cast<LoadInst>(&inst)) {
                if (loadInst->getPointerOperand() == arg) {
                    Align alignment = loadInst->getAlign();
                    if (alignment < minAlignment) {
                        minAlignment = alignment.value();
                    }
                }
            }

            // If the instruction is a store instruction
            // and the pointer operand is the argument
            // Do the same
            if (auto *storeInst = dyn_cast<StoreInst>(&inst)) {
                if (storeInst->getPointerOperand() == arg) {
                    Align alignment = storeInst->getAlign();
                    if (alignment < minAlignment) {
                        minAlignment = alignment.value();
                    }
                }
            }
        }
    }
    return Log2_64(minAlignment);
}

PreservedAnalyses BitTheftPass::run(Module &M, ModuleAnalysisManager &AM) {
    errs() << "Module Pass: " << M.getName() << '\n';
    for (auto &function : M.functions()) {
        if (function.getName().startswith("llvm.")) {
            continue;
        }
        errs() << function.getName() << ": "
               << Function::isInternalLinkage(function.getLinkage()) << '\n';
    }
    return PreservedAnalyses::all();
}

} // namespace llvm
