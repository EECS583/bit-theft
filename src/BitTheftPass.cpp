#include "BitTheftPass.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>

#include <algorithm>
#include <ranges>

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

std::unordered_map<Argument *, std::vector<Argument *>>
BitTheftPass::matching(std::unordered_map<Argument *, uint64_t> ptrCandidates,
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

auto BitTheftPass::getCandidateCalleeFunctions(Module &M) {
    return M.functions() | std::views::filter([](const Function &F) {
               return Function::isInternalLinkage(F.getLinkage()) &&
                      find_if(F.args(),
                              [](const Argument &argument) {
                                  return argument.getType()->isPointerTy();
                              }) != F.args().end() &&
                      !F.hasFnAttribute(Attribute::AttrKind::NoInline);
           });
}

auto BitTheftPass::getCandidateCallerFunctions(Module &M) {
    return M.functions() | std::views::filter([](const Function &F) {
               return !F.isIntrinsic();
           });
}

std::optional<Align> BitTheftPass::getPointerAlignByUser(const Value &V) {
    if (!V.getType()->isPointerTy())
        return std::nullopt;
    auto alignments =
        V.users() |
        std::views::transform([&V](const User *U) -> std::optional<Align> {
            const auto *I = dyn_cast<Instruction>(U);
            if (I == nullptr)
                return std::nullopt;
            switch (I->getOpcode()) {
            case Instruction::Load: {
                const auto *load = dyn_cast<LoadInst>(I);
                return (load->getPointerOperand() == &V)
                           ? std::make_optional(load->getAlign())
                           : std::nullopt;
            }
            case Instruction::Store: {
                const auto *store = dyn_cast<StoreInst>(I);
                return (store->getPointerOperand() == &V)
                           ? std::make_optional(store->getAlign())
                           : std::nullopt;
            }
            case Instruction::GetElementPtr: {
                const auto *getElementPtr = dyn_cast<GetElementPtrInst>(I);
                return (getElementPtr->getPointerOperand() == &V)
                           ? BitTheftPass::getPointerAlignByUser(*getElementPtr)
                           : std::nullopt;
            }
            case Instruction::PHI:
                return BitTheftPass::getPointerAlignByUser(*I);
            default:
                return std::nullopt;
            }
        }) |
        std::views::filter(
            [](std::optional<Align> align) { return align.has_value(); }) |
        std::views::transform(
            [](std::optional<Align> align) { return align.value(); });
    return std::ranges::fold_left_first(alignments,
                                        [](Align accumulator, Align align) {
                                            return std::min(accumulator, align);
                                        });
}

PreservedAnalyses BitTheftPass::run(Module &M, ModuleAnalysisManager &AM) {
    for (const auto &F : BitTheftPass::getCandidateCalleeFunctions(M)) {
        errs() << F.getName() << ": \n";
        for (const auto &argument : F.args()) {
            auto maybe_align = BitTheftPass::getPointerAlignByUser(argument);
            if (maybe_align.has_value())
                errs() << maybe_align.value().value() << '\n';
        }
    }
    return PreservedAnalyses::all();
}

} // namespace llvm
