//===-- memcpy.c ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// memcpy wrapper
//
//===----------------------------------------------------------------------===//

#include "common.h"

void *__real_memcpy(void *dest, const void *src, size_t n);

void *__wrap_memcpy(void *dest, const void *src, size_t n);
void *__wrap_memcpy(void *dest, const void *src, size_t n) {
  // printf("memcpy(dest=%p, src=%p, n=%zu)\n", dest, src, n);
  return __real_memcpy(dest, src, n);
}

