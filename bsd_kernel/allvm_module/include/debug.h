//===-- debug.h -----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Debugging and tracing functionality.
// Based on LLVM's Debugging.h
// No DEBUG_TYPE support yet, since not sure needed.
//
//===----------------------------------------------------------------------===//

#include "common.h"

#ifndef NDEBUG
extern char __DebugEnabled;

#define DEBUG(X) \
  do { \
    if (__DebugEnabled) { X; } \
  } while(0)

#else // ENABLE_DEBUG

#define DEBUG(X) do{}while(0)

#endif // ENABLE_DEBUG

#define TRACE() DEBUG(printf("<TRACE> Call to '%s'\n", __FUNCTION))

#endif // _DEBUG_H_


