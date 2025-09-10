#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Config/llvm-config.h" // For LLVM_VERSION_MAJOR macro detection

using namespace llvm;

namespace {
class InstrumentMcMiniGlobalsPass : public PassInfoMixin<InstrumentMcMiniGlobalsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    LLVMContext &Ctx = M.getContext();

#if LLVM_VERSION_MAJOR >= 15
    Type *PtrTy = PointerType::get(Ctx, 0);
#else
    Type *PtrTy = Type::getInt8PtrTy(Ctx);
#endif

    FunctionCallee checkReadFn = M.getOrInsertFunction(
        "mcmini_read", Type::getVoidTy(Ctx), PtrTy, PtrTy);

    FunctionCallee checkWriteFn = M.getOrInsertFunction(
        "mcmini_write", Type::getVoidTy(Ctx), PtrTy, PtrTy);

    bool Changed = false;
    SmallVector<GlobalVariable *, 16> Globals;
    for (GlobalVariable &GV : M.globals())
      Globals.push_back(&GV);

    auto &FAMProxy = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M);
    FunctionAnalysisManager &FAM = FAMProxy.getManager();

    for (Function &F : M) {
      if (F.isDeclaration())
        continue;

      AliasAnalysis &AA = FAM.getResult<AAManager>(F);
      for (Instruction &I : instructions(F)) {
        IRBuilder<> Builder(&I);
        if (auto *LI = dyn_cast<LoadInst>(&I)) {
          Value *Ptr = LI->getPointerOperand();
          GlobalVariable *GlobalFound = findAliasedGlobal(Ptr, Globals, AA);
          if (GlobalFound) {
            Builder.SetInsertPoint(LI);
            Constant *varName = Builder.CreateGlobalStringPtr(GlobalFound->getName());
	    Value *ptrCast = Builder.CreateBitCast(Ptr, PtrTy);
            Builder.CreateCall(checkReadFn, {ptrCast, varName});
            Changed = true;
          }
        } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
          Value *Ptr = SI->getPointerOperand();
          GlobalVariable *GlobalFound = findAliasedGlobal(Ptr, Globals, AA);
          if (GlobalFound) {
            Builder.SetInsertPoint(SI);
            Constant *varName = Builder.CreateGlobalStringPtr(GlobalFound->getName());
            Value *ptrCast = Builder.CreateBitCast(Ptr, PtrTy);
            Builder.CreateCall(checkWriteFn, {ptrCast, varName});
            Changed = true;
          }
        }
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  GlobalVariable *findAliasedGlobal(Value *Ptr, ArrayRef<GlobalVariable *> Globals, AliasAnalysis &AA) {
    for (GlobalVariable *GV : Globals) {
      AliasResult AR = AA.alias(Ptr, GV);
      if (AR != AliasResult::NoAlias) {
        return GV;
      }
    }
    return nullptr;
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "InstrumentMcMiniGlobalsPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "instrument-mcmini-globals") {
                    MPM.addPass(InstrumentMcMiniGlobalsPass());
                    return true;
                  }
                  return false;
                });
          }};
}

