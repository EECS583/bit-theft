#include "BitTheftPass.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>

#define DEBUG_TYPE "bit-theft"

namespace llvm {

std::vector<Argument *> BitTheftPass::getBitTheftCandidate(Function &F) {
    std::vector<Argument *> candidates;
    for (auto &arg : F.args()) {
        if (auto *intType = dyn_cast<IntegerType>(arg.getType())) {
            if (intType->getBitWidth() % 8 != 0) {
                candidates.push_back(&arg);
            }
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

std::unordered_map<Argument *, std::vector<Argument *>> BitTheftPass::matching(
    std::unordered_map<Argument *, uint64_t> ptrCandidates,
    std::vector<Argument *> intCandidates) {
        std::unordered_map<Argument *, std::vector<Argument *>> matches;
        for (auto &intArg : intCandidates) {
            for (auto &ptrArg : ptrCandidates) {
                if (ptrArg.second >= intArg->getType()->getIntegerBitWidth()) {
                    matches[ptrArg.first].push_back(intArg);
                    ptrArg.second -= intArg->getType()->getIntegerBitWidth();
                }
            }
        }
        return matches;
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
