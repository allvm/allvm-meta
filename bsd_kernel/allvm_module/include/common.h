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
#define START_DECLS extern "C" {
#define END_DECLS }
#else
#define START_DECLS
#define END_DECLS
#endif

START_DECLS

#include <sys/types.h>
#include <sys/module.h>

void panic(const char *, ...) __dead2 __printflike(1, 2);
int printf(const char *, ...) __printflike(1, 2);

END_DECLS

static inline const char* nullstr(const char *str) {
  return str ? str : "<null>";
}

#define UNSUPPORTED(name) \
  void name(void); \
  void name() { printf("Unsupported call to '%s'!\n", __FUNCTION__); }

#endif // _COMMON_H_
