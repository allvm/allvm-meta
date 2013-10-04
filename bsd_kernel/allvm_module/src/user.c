//===-- user.c ------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Userspace test program
//
//===----------------------------------------------------------------------===//

#include <sys/types.h>
#include "allvm.h"

// Kernel bytecode

#include <stdio.h>
#include <stdlib.h>

// Provided by kernel.bc.o
extern char _binary_kernel_bc_start;
extern char _binary_kernel_bc_end;

static const char *start = &_binary_kernel_bc_start;
static const char *end = &_binary_kernel_bc_end;

int main(int argc, char **argv) {
  void *jit_handle = createJIT(start, end, 0);
  printf("jit_handle: %p\n", jit_handle);

  return 0;
}
