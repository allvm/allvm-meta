//===-- assert.h ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Definition of assert() macro, based on <assert.h>
// (without hooks to NDEBUG)
//
//===----------------------------------------------------------------------===//

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include "common.h"

START_DECLS

void __assert(const char *, const char *, int, const char *) __dead2;

#define assert(e) \
  ((e) ? (void)0 : __assert(__func__, __FILE__, __LINE__, #e))

END_DECLS

#endif // _ASSERT_H_
