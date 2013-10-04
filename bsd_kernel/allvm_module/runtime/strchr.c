//===-- strchr.c ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// strchr implementation (forward to index()).
//
//===----------------------------------------------------------------------===//

char *index(const char *, int);
char *strchr(const char* c, int n);

char *strchr(const char* c, int n) {
  return index(c, n);
}

