//===-- bsd.c -------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Wrappers for bcopy/bzero
//
//===----------------------------------------------------------------------===//

#include "common.h"
#include "debug.h"

void __real_bzero(void *s, size_t n);
void __real_bcopy(const void *src, void *dest, size_t n);

void __wrap_bzero(void *s, size_t n);
void __wrap_bzero(void *s, size_t n) {
  DEBUG(printf("bzero(s=%p, n=%zu)\n", s, n));
  return __real_bzero(s, n);
}

void __wrap_bcopy(const void *src, void *dest, size_t n);
void __wrap_bcopy(const void *src, void *dest, size_t n) {
  DEBUG(printf("bcopy(src=%p, dest=%p, n=%zu)\n", src, dest, n));
  return __real_bcopy(src, dest, n);
}

