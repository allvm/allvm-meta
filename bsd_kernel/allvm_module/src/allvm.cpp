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
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include "allvm.h"

using namespace llvm;

class JITContext {
  OwningPtr<LLVMContext> Context;
  Module *M;
  OwningPtr<ExecutionEngine> EE;
public:
  JITContext(LLVMContext *Context, Module *M, ExecutionEngine *EE)
    : Context(Context), M(M), EE(EE) {}
  ~JITContext() {
    // EE->freeMachineCodeForFunction(...);
  }
};

void *createJIT(const void *bc_start, const void *bc_end, char lazy) {
  outs() << "Creating JIT for BC from " << bc_start << " to " << bc_end << "\n";
  OwningPtr<LLVMContext> C(new LLVMContext());

  const char *start = (const char *)bc_start, *end = (const char *)bc_end;
  OwningPtr<MemoryBuffer> BCBuffer(
    MemoryBuffer::getMemBuffer(StringRef(start, end-start), "bckernel", false));
  if (!BCBuffer) {
    errs() << "Failed to construct memory buffer\n";
    return NULL;
  }


  std::string ErrMsg;
  Module *M;
  if (lazy) {
    outs() << "Creating lazily parsed module...\n";
    M = getLazyBitcodeModule(&*BCBuffer, *C, &ErrMsg);
  } else {
    errs() << "Error parsing bitcode: " << ErrMsg << "\n";
    M = ParseBitcodeFile(&*BCBuffer, *C, &ErrMsg);
  }
  if (!M) {
    return NULL;
  }

  outs() << "Constructing execution engine...\n";
  EngineBuilder Builder(M);
  Builder.setCodeModel(CodeModel::Kernel);
  Builder.setRelocationModel(Reloc::Static);
  Builder.setUseMCJIT(false);
  Builder.setErrorStr(&ErrMsg);

  ExecutionEngine *EE = Builder.create();
  if (!EE) {
    errs() << "Error creating execution engine: " << ErrMsg << "\n";
    return NULL;
  }

  return new JITContext(C.take(), M, EE);
}

void *createFunction(void *JIT, const char *name) {
  assert(JIT);
  JITContext *JC = static_cast<JITContext *>(JIT);

  return NULL;
}

void destroyJIT(void *JIT) {
  assert(JIT);
  delete static_cast<JITContext *>(JIT);
}

static void __attribute__((constructor)) init() {
  InitializeNativeTarget();
}

static void __attribute__((destructor)) fini() {
  llvm_shutdown();
}
