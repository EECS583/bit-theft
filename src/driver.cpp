#include "BitTheftPass.h"

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

using namespace llvm;

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
    return {.APIVersion = LLVM_PLUGIN_API_VERSION,
            .PluginName = "BitTheftPass",
            .PluginVersion = "v0.1",
            .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef name, ModulePassManager &MPM,
                       [[maybe_unused]] ArrayRef<PassBuilder::PipelineElement>
                           elements) {
                        if (name == "bit-theft") {
                            MPM.addPass(BitTheftPass());
                            MPM.addPass(ModuleInlinerWrapperPass());
                            return true;
                        }
                        return false;
                    });
            }};
}
