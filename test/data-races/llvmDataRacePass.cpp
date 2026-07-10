#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

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
        "mcmini_write", Type::getVoidTy(Ctx), PtrTy, PtrTy, Type::getInt64Ty(Ctx));

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

      for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
          if (auto *LI = dyn_cast<LoadInst>(&I)) {
            Value *ptr = LI->getPointerOperand();
            GlobalVariable *GV = findTrackedGlobal(ptr, Globals, AA);
            if (GV)
              Changed |= instrumentGlobalRead(LI, GV, checkReadFn, PtrTy);
          } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
            Value *ptr = SI->getPointerOperand();
            GlobalVariable *GV = findTrackedGlobal(ptr, Globals, AA);
            if (GV)
              Changed |= instrumentGlobalWrite(SI, GV, checkWriteFn, PtrTy);
          }
        }
      }
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  static GlobalVariable *findTrackedGlobal(
      Value *ptr, ArrayRef<GlobalVariable *> Globals, AliasAnalysis &AA) {
    Value *base = ptr->stripPointerCasts();
    while (auto *GEP = dyn_cast<GEPOperator>(base)) {
      base = GEP->getPointerOperand()->stripPointerCasts();
    }
    if (auto *GV = dyn_cast<GlobalVariable>(base)) {
      for (GlobalVariable *Candidate : Globals) {
        if (GV == Candidate)
          return GV;
      }
    }

    for (GlobalVariable *GV : Globals) {
      AliasResult AR = AA.alias(ptr, GV);
      if (AR != AliasResult::NoAlias) {
        return GV;
      }
    }
    return nullptr;
  }

  static Value *normalizeIntegerValue(IRBuilder<> &Builder, Value *value) {
    Type *ty = value->getType();
    if (!ty->isIntegerTy())
      return nullptr;
    return Builder.CreateZExtOrTrunc(value, Builder.getInt64Ty());
  }

  static bool instrumentGlobalWrite(
      StoreInst *SI, GlobalVariable *GV,
      FunctionCallee writeFn, Type *PtrTy) {
    IRBuilder<> Builder(SI);

    Value *storeValue = SI->getValueOperand();
    Value *normalizedValue = normalizeIntegerValue(Builder, storeValue);
    if (!normalizedValue)
      return false;

    Value *ptrCast = Builder.CreateBitCast(
        SI->getPointerOperand(), PtrTy);
    Value *name = Builder.CreateGlobalStringPtr(GV->getName());

    Builder.CreateCall(writeFn, {ptrCast, name, normalizedValue});
    return true;
  }

  static bool instrumentGlobalRead(
      LoadInst *LI, GlobalVariable *GV,
      FunctionCallee readFn, Type *PtrTy) {
    Instruction *insertPt = LI->getNextNode();
    if (!insertPt)
      insertPt = LI->getParent()->getTerminator();
    IRBuilder<> Builder(insertPt);

    Value *ptrCast = Builder.CreateBitCast(
        LI->getPointerOperand(), PtrTy);
    Value *name = Builder.CreateGlobalStringPtr(GV->getName());

    Builder.CreateCall(readFn, {ptrCast, name});
    return true;
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

