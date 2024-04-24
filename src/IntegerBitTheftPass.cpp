#include "IntegerBitTheftPass.h"

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
#include <llvm/Transforms/Utils/Cloning.h>

#include <numeric>
#include <ranges>

#define DEBUG_TYPE "integer-bit-theft"

namespace llvm {

bool IntegerBitTheftPass::isThief(const Value &V) {
    return V.getType()->isIntegerTy() &&
           V.getType()->getIntegerBitWidth() % 8 != 0;
}

bool IntegerBitTheftPass::isCandidateCalleeFunction(const Function &F) {
    return !F.isIntrinsic() && !F.isDeclaration() && !F.hasOptNone() &&
           std::all_of(F.users().begin(), F.users().end(),
                       [](const User *U) {
                           return dyn_cast<CallInst>(U) != nullptr;
                       }) &&
           count_if(F.args(), isThief) > 1;
}

SmallVector<Function *>
IntegerBitTheftPass::createCandidateFunctionSnapshot(Module &M) {
    auto fs = M.functions() | std::views::filter(isCandidateCalleeFunction) |
              std::views::transform([](Function &F) { return &F; });
    return SmallVector<Function *>(fs.begin(), fs.end());
}

std::pair<Function *, SmallVector<unsigned int>>
IntegerBitTheftPass::createTransformedFunction(Function &F) {
    auto thiefBits =
        F.args() | std::views::filter(isThief) |
        std::views::transform([](const Argument &argument) -> unsigned int {
            return argument.getType()->getIntegerBitWidth();
        });
    auto thiefBitsSum = std::reduce(thiefBits.begin(), thiefBits.end());

    SmallVector<Type *> argumentTypes;
    argumentTypes.push_back(IntegerType::get(F.getContext(), thiefBitsSum));

    SmallVector<unsigned int> mappedArgNo(
        F.arg_size(), static_cast<unsigned int>(F.arg_size()));
    for (const Argument &argument : F.args()) {
        if (isThief(argument)) {
            mappedArgNo[argument.getArgNo()] = 0;
        } else {
            mappedArgNo[argument.getArgNo()] =
                static_cast<unsigned int>(argumentTypes.size());
            argumentTypes.push_back(argument.getType());
        }
    }

    auto *FTy =
        FunctionType::get(F.getReturnType(), argumentTypes, F.isVarArg());
    auto *transformed =
        Function::Create(FTy, F.getLinkage(), "", F.getParent());
    transformed->setLinkage(GlobalValue::InternalLinkage);
    transformed->setCallingConv(CallingConv::Fast);
    transformed->setName(F.getName().str() + ".integer_bit_theft");

    auto *prefix = BasicBlock::Create(F.getContext(), "", transformed);
    ValueToValueMapTy VMap;
    IRBuilder<> builder(prefix);
    unsigned int offset = 0;
    for (const Argument &argument : F.args() | std::views::filter(isThief)) {
        auto *merged = transformed->getArg(0);
        auto *shifted =
            offset == 0 ? merged : builder.CreateAShr(merged, offset);
        auto *truncated = builder.CreateTrunc(shifted, argument.getType());
        VMap[&argument] = truncated;
        offset += argument.getType()->getIntegerBitWidth();
    }

    for (const auto &argument : F.args()) {
        if (VMap.count(&argument) == 0) {
            VMap[&argument] =
                transformed->getArg(mappedArgNo[argument.getArgNo()]);
        }
    }

    SmallVector<ReturnInst *, 8> _;
    CloneFunctionInto(transformed, &F, VMap,
                      CloneFunctionChangeType::LocalChangesOnly, _,
                      ".integer_bit_theft");
    builder.CreateBr(dyn_cast<BasicBlock>(VMap[&(F.getEntryBlock())]));
    return std::make_pair(transformed, mappedArgNo);
}

PreservedAnalyses
IntegerBitTheftPass::run(Module &M,
                         [[maybe_unused]] ModuleAnalysisManager &AM) {
    auto candidates = createCandidateFunctionSnapshot(M);
    for (Function *F : candidates) {
        auto &&[transformed, _] =
            IntegerBitTheftPass::createTransformedFunction(*F);
        SmallVector<User *> users(F->users());
        for (auto *U : users) {
            auto *I = dyn_cast<CallInst>(U);
            if (I->getParent()->getParent()->hasOptNone())
                continue;
            auto *ty = transformed->getArg(0)->getType();
            SmallVector<Value *> embeddedArgs;
            unsigned int offset = 0;
            Value *merged = ConstantInt::get(ty, 0);
            for (const Argument &argument :
                 F->args() | std::views::filter(isThief)) {
                auto *extended = new ZExtInst(
                    I->getArgOperand(argument.getArgNo()), ty, "", I);
                auto *shifted =
                    offset == 0
                        ? static_cast<Value *>(extended)
                        : BinaryOperator::CreateShl(
                              extended, ConstantInt::get(ty, offset), "", I);
                merged = BinaryOperator::CreateOr(merged, shifted, "", I);
                offset += argument.getType()->getIntegerBitWidth();
            }
            embeddedArgs.push_back(merged);
            for (const Argument &argument : F->args()) {
                if (!isThief(argument))
                    embeddedArgs.push_back(
                        I->getArgOperand(argument.getArgNo()));
            }
            auto *call = CallInst::Create(transformed, embeddedArgs, "", I);
            I->replaceAllUsesWith(call);
            I->eraseFromParent();
        }
    }
    return PreservedAnalyses::none();
}

} // namespace llvm
