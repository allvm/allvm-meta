//===-- gcc.c -------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// libgcc stubs
//
//===----------------------------------------------------------------------===//

#include "common.h"
#include "debug.h"

void __register_frame(void *);
void __deregister_frame(void*);

void __register_frame(void *unused) {
  TRACE();
}

void __deregister_frame(void *unused) {
  TRACE();
}
