//===-- jit.cpp -----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// ALLVM JIT example.
//
//===----------------------------------------------------------------------===//

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

#include "llvm/CodeGen/LinkAllCodegenComponents.h"

#include <vector>

using namespace llvm;

// Used in main.c
extern "C" int testJIT(char go);


// Test code taken from "HowToUseJIT.cpp" example in LLVM tree
Function *populateTestModule(Module *M, LLVMContext &C) {
  // Create the add1 function entry and insert this entry into module M.  The
  // function will have a return type of "int" and take an argument of "int".
  // The '0' terminates the list of argument types.
  Function *Add1F =
    cast<Function>(M->getOrInsertFunction("add1", Type::getInt32Ty(C),
                                          Type::getInt32Ty(C),
                                          (Type *)0));

  // Add a basic block to the function. As before, it automatically inserts
  // because of the last argument.
  BasicBlock *BB = BasicBlock::Create(C, "EntryBlock", Add1F);

  // Create a basic block builder with default parameters.  The builder will
  // automatically append instructions to the basic block `BB'.
  IRBuilder<> builder(BB);

  // Get pointers to the constant `1'.
  Value *One = builder.getInt32(1);

  // Get pointers to the integer argument of the add1 function...
  assert(Add1F->arg_begin() != Add1F->arg_end()); // Make sure there's an arg
  Argument *ArgX = Add1F->arg_begin();  // Get the arg
  ArgX->setName("AnArg");            // Give it a nice symbolic name for fun.

  // Create the add instruction, inserting it into the end of BB.
  Value *Add = builder.CreateAdd(One, ArgX);

  // Create the return instruction and add it to the basic block
  builder.CreateRet(Add);

  // Now, function add1 is ready.


  // Now we're going to create function `foo', which returns an int and takes no
  // arguments.
  Function *FooF =
    cast<Function>(M->getOrInsertFunction("foo", Type::getInt32Ty(C),
                                          (Type *)0));

  // Add a basic block to the FooF function.
  BB = BasicBlock::Create(C, "EntryBlock", FooF);

  // Tell the basic block builder to attach itself to the new basic block
  builder.SetInsertPoint(BB);

  // Get pointer to the constant `10'.
  Value *Ten = builder.getInt32(10);

  // Pass Ten to the call to Add1F
  CallInst *Add1CallRes = builder.CreateCall(Add1F, Ten);
  Add1CallRes->setTailCall(true);

  // Create the return instruction and add it to the basic block.
  builder.CreateRet(Add1CallRes);

  return FooF;
}
int testJIT(char go) {
  printf("testJIT() entry\n");
  InitializeNativeTarget();

  LLVMContext Context;

  Module *M = new Module("test", Context);

  Function *MainF = populateTestModule(M, Context);

  // Now we create the JIT.
  EngineBuilder Builder(M);
  Builder.setCodeModel(CodeModel::Kernel);
  Builder.setRelocationModel(Reloc::Static);
  Builder.setUseMCJIT(false);
  ExecutionEngine* EE = Builder.create();
  std::vector<GenericValue> noargs;
  intptr_t ptr = (intptr_t)EE->getPointerToFunction(MainF);
  GenericValue gv = EE->runFunction(MainF, noargs);
  EE->freeMachineCodeForFunction(MainF);
  delete EE;

  // XXX: Eventually be good about calling llvm_shutdown() ?

  return gv.IntVal.getZExtValue();
}
