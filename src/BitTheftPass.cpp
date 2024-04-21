#include "BitTheftPass.h"
#include <cstdint>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>
#include <vector>

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

Matching
BitTheftPass::matching(std::unordered_map<Argument *, uint64_t> ptrCandidates,
                       std::vector<Argument *> intCandidates) {
    Matching matches;
    std::vector<bool> visited(intCandidates.size(), false);
    for (auto &ptrCandidate : ptrCandidates) {
        NewArg newArg;
        uint64_t size = 64 - ptrCandidate.second;
        newArg.emplace_back(size, ptrCandidate.first->getArgNo());
        for (size_t i = 0; i < intCandidates.size(); i++) {
            if (visited[i]) {
                continue;
            }
            if (size >= intCandidates[i]->getType()->getIntegerBitWidth()) {
                newArg.emplace_back(
                    intCandidates[i]->getType()->getIntegerBitWidth(),
                    intCandidates[i]->getArgNo());
                visited[i] = true;
                size += intCandidates[i]->getType()->getIntegerBitWidth();
            }
        }
        matches.push_back(newArg);
    }
    for (auto &intCandidate : intCandidates) {
        if (!visited[intCandidate->getArgNo()]) {
            NewArg newArg;
            newArg.emplace_back(intCandidate->getType()->getIntegerBitWidth(),
                                intCandidate->getArgNo());
            matches.push_back(newArg);
        }
    }

    return matches;
}

bool BitTheftPass::isCandidateCalleeFunction(const Function &F) {
    return F.hasInternalLinkage() && !F.isIntrinsic() && !F.isDeclaration() &&
           find_if(F.args(), [](const Argument &argument) {
               return argument.getType()->isPointerTy();
           }) != F.args().end();
}

bool BitTheftPass::isCandidateCallerFunction(const Function &F) {
    return !F.isIntrinsic() && !F.isDeclaration();
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

std::vector<Argument *> BitTheftPass::getOthers(Function &F, Matching matches) {
    std::vector<Argument *> others;
    std::vector<bool> visited(F.arg_size(), false);
    for (auto &match : matches) {
        for (auto &element : match) {
            visited[element.original_ind] = true;
        }
    }
    for (size_t i = 0; i < F.arg_size(); i++) {
        if (!visited[i]) {
            others.push_back(F.getArg(i));
        }
    }
    return others;
}

void BitTheftPass::embedAtCaller(CallInst *callInst, Function *caller,
                                 Function *callee, Matching matches,
                                 std::vector<Argument *> others) {
    std::vector<Value *> embeddedArgs;

    for (size_t i = 0; i < matches.size(); i++) {
        auto &match = matches[i];
        uint64_t ptrArgNo = match[0].original_ind;
        uint64_t availableLSB = 0;
        embeddedArgs[i] = callInst->getArgOperand(ptrArgNo);
        for (size_t j = 1; j < match.size(); j++) {
            uint64_t intArgNo = match[j].original_ind;
            uint64_t intArgBits = match[j].size;
            BinaryOperator *shl = BinaryOperator::Create(
                Instruction::Shl, callInst->getArgOperand(intArgNo),
                ConstantInt::get(Type::getInt16Ty(caller->getContext()),
                                 availableLSB),
                "shl", callInst);
            BinaryOperator *orInst = BinaryOperator::Create(
                Instruction::Or, embeddedArgs[i], shl, "or", callInst);
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

FunctionType *BitTheftPass::getEmbeddedFuncTy(Function &F, Matching matches,
                                              std::vector<Argument *> others,
                                              LLVMContext &C) {
    std::vector<Type *> argTypes;
    for (auto &match : matches) {
        argTypes.push_back(IntegerType::get(C, 64));
    }
    for (auto &other : others) {
        argTypes.push_back(other->getType());
    }

    Type *retType = F.getReturnType();
    bool isVarArg = F.isVarArg();
    return FunctionType::get(retType, argTypes, isVarArg);
}

Function *BitTheftPass::getEmbeddedFunc(Function &F, FunctionType *FTy,
                                        StringRef name, Matching matches,
                                        std::vector<Argument *> others) {
    Function *newFunc =
        Function::Create(FTy, F.getLinkage(), name, F.getParent());
    BasicBlock *entry = BasicBlock::Create(F.getContext(), "entry", newFunc);
    IRBuilder<> builder(entry);
    std::vector<Value *> args(F.arg_size());
    for (size_t i = 0; i < matches.size(); i++) {
        auto &match = matches[i];
        uint64_t ptrArgNo = match[0].original_ind;
        args[ptrArgNo] = builder.CreateTrunc(newFunc->arg_begin() + i,
                                             F.getArg(ptrArgNo)->getType());
        size_t alignment = (1 << (match[0].size)) - 1;
        size_t mask = alignment << (64 - match[0].size);
        args[ptrArgNo] = builder.CreateAnd(
            args[ptrArgNo],
            ConstantInt::get(Type::getInt64Ty(F.getContext()), mask));
        Value *var = newFunc->arg_begin() + i;
        for (size_t j = 1; j < match.size(); j++) {
            uint64_t intArgNo = match[j].original_ind;
            uint64_t intArgBits = match[j].size;
            Value *trunc =
                builder.CreateTrunc(var, F.getArg(intArgNo)->getType());
            var = builder.CreateAShr(trunc, intArgBits);
            args[intArgNo] = trunc;
        }
    }
    Value *callInst = builder.CreateCall(&F, args);
    builder.CreateRet(callInst);
    return newFunc;
}

PreservedAnalyses BitTheftPass::run(Module &M, ModuleAnalysisManager &AM) {
    errs() << "Module Pass: " << M.getName() << '\n';
    for (auto &function : M.functions()) {
        if (function.isIntrinsic() || function.isDeclaration()) {
            continue;
        }
        std::vector<Argument *> intCandidates = getBitTheftCandidate(function);
        std::unordered_map<Argument *, uint64_t> ptrCandidates =
            getBitTheftCandidatePtr(function);
        Matching matches = matching(ptrCandidates, intCandidates);
        std::vector<Argument *> others = getOthers(function, matches);
        Function *newFunc = getEmbeddedFunc(
            function,
            getEmbeddedFuncTy(function, matches, others, function.getContext()),
            function.getName().str() + "_embedded", matches, others);
        for (const auto &user : function.users()) {
            if (auto *callInst = dyn_cast<CallInst>(user)) {
                embedAtCaller(callInst, &function, newFunc, matches, others);
            }
        }
    }
    return PreservedAnalyses::all();
}

} // namespace llvm
