//===-- allvm.h -----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Header for primary ALLVM functionality.
//
//===----------------------------------------------------------------------===//

#ifndef _ALLVM_H_
#define _ALLVM_H_

#include "common.h"

START_DECLS

// Initialize the ALLVM jit the specified bytecode
void *createJIT(const void* bc_start, const void* bc_end, char lazy);

// Request a function be JIT-compiled and return pointer to it
void *createFunction(void *JIT, const char *name);

// Cleanup the allocated JIT from createJIT().
void destroyJIT(void *JIT);

END_DECLS

#endif // _ALLVM_H_
