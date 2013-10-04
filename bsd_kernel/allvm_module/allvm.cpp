//===-- allvm.cpp ---------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Primary ALLVM interface exposed to kernel module.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

#include "allvm.h"

class JITContext {
  LLVMContext *Context;
  Module *M;
  ExecutionEngine *EE;

  JITContext(LLVMContext *Context, Module *M, ExecutionEngine *EE)
    : Context(Context), M(M), EE(EE) {}
public:
  JITContext *createWithBC(ArrayRef<const char*> BC) {
    OwningPtr<LLVMContext> C(new LLVMContext());

    // If error, return null


    return new JITContext(C.take(), M, EE);
  }
  initWith(Module *M) {
    this->M = M;
    EngineBuilder Builder(M);
    Builder.setCodeModel(CodeModel::Kernel);
    Builder.setRelocationModel(Reloc::Static);
    Builder.setUseMCJIT(false);

    EE = Builder.create();
    assert(EE);
  }

  ~JITContext() {
    // EE->freeMachineCodeForFunction(...);
    delete EE;

  }
};

void *createJIT(const void *bc_start, const void *bc_end) {
  JITContext *JC = new JITContext(bc_start, bc_end);
  if (!JC->init()) {
    delete JC;
    return 0;
  }
  return JC;
}

void *createFunction(void *JIT, const char *name) {
  assert(JIT);
  JITContext *JC = static_cast<JITContext *>(JIT);
}

void destroyJIT(void *JIT) {
  assert(JIT);
  delete static_cast<JITContext *>(JIT);
}

void init() __attribute__((constructor)) {
  InitializeNativeTarget();
}

void fini() __attribute__((destructor)) {
  llvm_shutdown();
}
