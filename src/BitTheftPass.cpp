#include "BitTheftPass.h"

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

#define DEBUG_TYPE "bit-theft"

namespace llvm {

BitTheftPass::Niche::Niche(const Argument &argument)
    : argument(&argument),
      align(BitTheftPass::getPointerAlign(
                argument.getParent()->getParent()->getDataLayout(), argument)
                .value_or(Align())) {}

const Argument *BitTheftPass::Niche::getArgument() const noexcept {
    return this->argument;
}

Align BitTheftPass::Niche::getAlign() const noexcept { return this->align; }

bool BitTheftPass::isCandidateCalleeFunction(const Function &F) {
    return !F.isIntrinsic() && !F.isDeclaration() && !F.hasOptNone() &&
           std::all_of(F.users().begin(), F.users().end(),
                       [](const User *U) {
                           return dyn_cast<CallInst>(U) != nullptr;
                       }) &&
           find_if(F.args(),
                   [](const Argument &argument) {
                       return argument.getType()->isPointerTy();
                   }) != F.args().end() &&
           find_if(F.args(),
                   [](const Argument &argument) {
                       return argument.getType()->isIntegerTy() &&
                              argument.getType()->getIntegerBitWidth() < 8;
                   }) != F.args().end() &&
           F.arg_size() >= 7;
}

SmallVector<Function *>
BitTheftPass::createCandidateFunctionSnapshot(Module &M) {
    auto fs = M.functions() | std::views::filter(isCandidateCalleeFunction) |
              std::views::transform([](Function &F) { return &F; });
    return SmallVector<Function *>(fs.begin(), fs.end());
}

std::optional<Align> BitTheftPass::getPointerAlign(const DataLayout &L,
                                                   const Value &V) {
    if (!V.getType()->isPointerTy())
        return std::nullopt;
    auto alignments =
        V.users() |
        std::views::transform([&L, &V](const User *U) -> std::optional<Align> {
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
                Align align =
                    L.getABITypeAlign(getElementPtr->getSourceElementType());
                return (getElementPtr->getPointerOperand() == &V)
                           ? std::make_optional(align)
                           : std::nullopt;
            }
            case Instruction::PHI:
                return BitTheftPass::getPointerAlign(L, *I);
            default:
                return std::nullopt;
            }
        }) |
        std::views::filter(
            [](std::optional<Align> align) { return align.has_value(); }) |
        std::views::transform(
            [](std::optional<Align> align) { return align.value(); });
    if (alignments.empty())
        return std::nullopt;
    else
        return std::reduce(alignments.begin(), alignments.end(),
                           *alignments.begin(),
                           [](Align accumulator, Align align) {
                               return std::min(accumulator, align);
                           });
}

SmallVector<BitTheftPass::BinPack>
BitTheftPass::getBinPackedNiche(const Function &F) {
    auto niches = F.args() | std::views::filter([](const Argument &argument) {
                      return argument.getType()->isPointerTy();
                  }) |
                  std::views::transform([](const Argument &argument) {
                      return BitTheftPass::Niche(argument);
                  }) |
                  std::views::filter([](const Niche &niche) {
                      return niche.getAlign().value() > 1;
                  });
    auto thieves = F.args() | std::views::filter([](const Argument &argument) {
                       const auto *intTy =
                           dyn_cast<IntegerType>(argument.getType());
                       return intTy != nullptr && intTy->getBitWidth() < 8;
                   });

    SmallVector<BinPack> bins;
    for (const auto &niche : niches)
        bins.emplace_back(niche, SmallVector<const Argument *>{});

    for (const Argument &thief : thieves) {
        auto bits_needed =
            static_cast<uint8_t>(thief.getType()->getIntegerBitWidth());
        for (auto &[niche, thieves] : bins) {
            auto thief_width =
                thieves |
                std::views::transform([](const Argument *argument) -> uint8_t {
                    return static_cast<uint8_t>(
                        argument->getType()->getIntegerBitWidth());
                });
            auto occupied_bits =
                std::reduce(thief_width.begin(), thief_width.end(),
                            static_cast<uint8_t>(0));
            if (bits_needed + occupied_bits <= Log2(niche.getAlign())) {
                thieves.push_back(&thief);
                break;
            }
        }
    }
    auto filtered = bins | std::views::filter([](const BinPack &pack) {
                        return !pack.second.empty();
                    });
    return SmallVector<BinPack>(filtered.begin(), filtered.end());
}

std::pair<Function *, SmallVector<unsigned int>>
BitTheftPass::createTransformedFunction(
    Function &F, const SmallVector<BitTheftPass::BinPack> &binPacks) {
    SmallVector<unsigned int> mappedArgNo(
        F.arg_size(), static_cast<unsigned int>(F.arg_size()));
    SmallVector<Type *> argumentTypes;
    for (const auto &[niche, thieves] : binPacks) {
        mappedArgNo[niche.getArgument()->getArgNo()] =
            static_cast<unsigned int>(argumentTypes.size());
        for (const auto *thief : thieves)
            mappedArgNo[thief->getArgNo()] =
                static_cast<unsigned int>(argumentTypes.size());
        argumentTypes.push_back(IntegerType::get(
            F.getContext(), F.getParent()->getDataLayout().getPointerSizeInBits(
                                F.getAddressSpace())));
    }
    for (const Argument &argument : F.args()) {
        if (mappedArgNo[argument.getArgNo()] == F.arg_size()) {
            mappedArgNo[argument.getArgNo()] =
                static_cast<unsigned int>(argumentTypes.size());
            argumentTypes.push_back(argument.getType());
        }
    }
    auto *FTy =
        FunctionType::get(F.getReturnType(), argumentTypes, F.isVarArg());
    auto *transformed =
        Function::Create(FTy, F.getLinkage(), "", F.getParent());
    transformed->setName(F.getName().str() + ".bit_theft");
    auto *prefix = BasicBlock::Create(F.getContext(), "", transformed);
    ValueToValueMapTy VMap;
    IRBuilder<> builder(prefix);
    for (const auto &[niche, thieves] : binPacks) {
        auto *casted =
            transformed->getArg(mappedArgNo[niche.getArgument()->getArgNo()]);
        auto *ptrInt =
            builder.CreateAnd(casted, ~(niche.getAlign().value() - 1));
        auto *ptr =
            builder.CreateIntToPtr(ptrInt, niche.getArgument()->getType());
        VMap[niche.getArgument()] = ptr;
        unsigned int offset = 0;
        for (const Argument *thief : thieves) {
            auto *shifted =
                offset == 0 ? casted : builder.CreateAShr(casted, offset);
            auto truncated = builder.CreateTrunc(shifted, thief->getType());
            offset += thief->getType()->getIntegerBitWidth();
            VMap[thief] = truncated;
        }
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
                      ".bit_theft");
    transformed->setLinkage(GlobalValue::InternalLinkage);
    builder.CreateBr(dyn_cast<BasicBlock>(VMap[&(F.getEntryBlock())]));
    return std::make_pair(transformed, mappedArgNo);
}

PreservedAnalyses
BitTheftPass::run(Module &M, [[maybe_unused]] ModuleAnalysisManager &AM) {
    auto candidates = createCandidateFunctionSnapshot(M);
    for (Function *F : candidates) {
        auto binPacks = BitTheftPass::getBinPackedNiche(*F);
        if (binPacks.empty())
            continue;
        auto &&[transformed, _] =
            BitTheftPass::createTransformedFunction(*F, binPacks);
        SmallVector<User *> users(F->users());

        for (auto *U : users) {
            auto *I = dyn_cast<CallInst>(U);
            if (I->getParent()->getParent()->hasOptNone())
                continue;
            SmallVector<Value *> embeddedArgs;
            for (const auto &[niche, thieves] : binPacks) {
                auto *ptr = I->getArgOperand(niche.getArgument()->getArgNo());
                auto *ptrIntegerTy = IntegerType::get(
                    F->getContext(),
                    F->getParent()->getDataLayout().getPointerSizeInBits(
                        ptr->getType()->getPointerAddressSpace()));
                Value *casted = new PtrToIntInst(ptr, ptrIntegerTy, "", I);
                unsigned int offset = 0;
                for (const Argument *thief : thieves) {
                    auto *extended =
                        new ZExtInst(I->getArgOperand(thief->getArgNo()),
                                     ptrIntegerTy, "", I);
                    auto *shifted =
                        offset == 0
                            ? static_cast<Value *>(extended)
                            : BinaryOperator::CreateShl(
                                  extended,
                                  ConstantInt::get(ptrIntegerTy, offset), "",
                                  I);
                    casted = BinaryOperator::CreateOr(casted, shifted, "", I);
                    offset += thief->getType()->getIntegerBitWidth();
                }
                embeddedArgs.push_back(casted);
            }
            for (const Argument &argument : F->args()) {
                if (find_if(binPacks,
                            [&argument](const BitTheftPass::BinPack &pack) {
                                return pack.first.getArgument() == &argument ||
                                       is_contained(pack.second, &argument);
                            }) == binPacks.end())
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
