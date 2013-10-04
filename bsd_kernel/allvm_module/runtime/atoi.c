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

long int strtol(const char *nptr, char **endptr, int base);

int atoi(const char *nptr);
int atoi(const char *nptr) {
  return strtol(nptr, 0, 10);
}

