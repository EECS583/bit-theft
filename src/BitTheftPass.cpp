#include "BitTheftPass.h"
#include <cstdint>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Value.h>
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

Matching BitTheftPass::matching(
    std::unordered_map<Argument *, uint64_t> ptrCandidates,
    std::vector<Argument *> intCandidates
    ) {
        Matching matches;
        std::vector<bool> visited(intCandidates.size(), false);
        for (auto & ptrCandidate : ptrCandidates){
            NewArg newArg;
            uint64_t size = 64 - ptrCandidate.second;
            newArg.emplace_back(size, ptrCandidate.first->getArgNo());
            for (size_t i = 0; i < intCandidates.size(); i++) {
                if (visited[i]) {
                    continue;
                }
                if (size >= intCandidates[i]->getType()->getIntegerBitWidth()) {
                    newArg.emplace_back(intCandidates[i]->getType()->getIntegerBitWidth(), intCandidates[i]->getArgNo());
                    visited[i] = true;
                    size += intCandidates[i]->getType()->getIntegerBitWidth();
                }
            }
            matches.push_back(newArg);
        }

        for (auto &intCandidate : intCandidates) {
            if (!visited[intCandidate->getArgNo()]) {
                NewArg newArg;
                newArg.emplace_back(intCandidate->getType()->getIntegerBitWidth(), intCandidate->getArgNo());
                matches.push_back(newArg);
            }
        }

        return matches;
}

void BitTheftPass::embedAtCaller(CallInst * callInst, Function* caller, Function * callee, Matching matches, std::vector<Argument *> others) {
    std::vector<Value *> embeddedArgs;

    for (size_t i = 0; i < matches.size(); i++) {
        auto &match = matches[i];
        uint64_t ptrArgNo = match[0].original_ind;
        uint64_t availableLSB = 0;
        embeddedArgs[i] = callInst->getArgOperand(ptrArgNo);
        for (size_t j = 1; j < match.size(); j++) {
            uint64_t intArgNo = match[j].original_ind;
            uint64_t intArgBits = match[j].size;
            BinaryOperator *shl = BinaryOperator::Create(Instruction::Shl, callInst->getArgOperand(intArgNo), ConstantInt::get(Type::getInt16Ty(caller->getContext()), availableLSB), "shl", callInst);
            BinaryOperator *orInst = BinaryOperator::Create(Instruction::Or, embeddedArgs[i], shl, "or", callInst);
            availableLSB += intArgBits;
            embeddedArgs[i] = orInst;
        }
    }
    for (auto &other : others) {
        embeddedArgs.push_back(other);
    }
    CallInst *newCallInst = CallInst::Create(callee, embeddedArgs, "");
    callInst->replaceAllUsesWith(newCallInst);
    callInst->eraseFromParent();
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
