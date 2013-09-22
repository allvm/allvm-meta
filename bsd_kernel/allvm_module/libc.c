//===-- libc.c ------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation of subset of libc needed for ALLVM-JIT
//
//===----------------------------------------------------------------------===//

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h> /* uprintf */

#define WARN() uprintf("Unsupported call to '%s'!\n", __FUNCTION__)

#define UNSUPPORTED(name) \
  void name(void); \
  void name() { WARN(); }

UNSUPPORTED(open);
UNSUPPORTED(close);

// TODO: Partially support these!
UNSUPPORTED(abort);
UNSUPPORTED(fputc);
UNSUPPORTED(fputs);
UNSUPPORTED(free);
UNSUPPORTED(fwrite);
UNSUPPORTED(malloc);
UNSUPPORTED(pthread_mutex_lock);
UNSUPPORTED(pthread_mutex_unlock);
UNSUPPORTED(pthread_once);
UNSUPPORTED(realloc);

void *__stderrp = 0;
