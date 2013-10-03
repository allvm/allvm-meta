//===-- signal.c ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Non-functional signal API implementation.
//
//===----------------------------------------------------------------------===//

#include "common.h"

int sigaction(void);
int sigaction(void) {
  return -1;
}

int sigemptyset(void);
int sigemptyset(void) {
  return -1;
}

int sigfillset(void);
int sigfillset(void) {
  return -1;
}

int sigprocmask(void);
int sigprocmask(void) {
  return -1;
}
