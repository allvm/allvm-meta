//===-- atoi.c ------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// atoi implementation (forwards to strtol)
//
//===----------------------------------------------------------------------===//

#include "debug.h"

long int strtol(const char *nptr, char **endptr, int base);

int atoi(const char *nptr);
int atoi(const char *nptr) {
  DEBUG(printf("atoi(str=%s)\n", nptr));
  return strtol(nptr, 0, 10);
}

