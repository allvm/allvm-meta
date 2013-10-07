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
#include "debug.h"

int sigaction(void);
int sigaction(void) {
  TRACE();
  return -1;
}

int sigemptyset(void);
int sigemptyset(void) {
  TRACE();
  return -1;
}

int sigfillset(void);
int sigfillset(void) {
  TRACE();
  return -1;
}

int sigprocmask(void);
int sigprocmask(void) {
  TRACE();
  return -1;
}
