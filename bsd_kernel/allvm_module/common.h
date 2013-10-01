//===-- common.h ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Common utility functions, macros, and constants for ALLVM.
//
//===----------------------------------------------------------------------===//

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/module.h>

void panic(const char *, ...) __dead2 __printflike(1, 2);
int printf(const char *, ...) __printflike(1, 2);

static inline const char* nullstr(const char *str) {
  return str ? str : "<null>";
}

//#define BAIL() panic("Unsupported call to '%s'!\n", __FUNCTION__)
#define BAIL() printf("Unsupported call to '%s'!\n", __FUNCTION__)

#define UNSUPPORTED(name) \
  void name(void); \
  void name() { BAIL(); }

#define TRACE() printf("<TRACE> Call to '%s'\n", __FUNCTION__)

#ifdef __cplusplus
}
#endif

#endif // _COMMON_H_
