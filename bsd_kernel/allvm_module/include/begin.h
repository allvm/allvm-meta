//===-- begin.h -----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// .ctors/.dtors support:
// * Put leading -1/-1 in beginning of each section
// * Define helpers for invoking ctors/dtors.
//
// Assumptions/Usage:
// * Include this in first/primary object file.
// * Call ctors/dtors appropriately in that file.
// * Link fin.o last to null-terminate the lists.
//
//===----------------------------------------------------------------------===//

#include "common.h"

typedef void (*func_ptr) (void);
static func_ptr __CTOR_LIST__[1]
  __attribute__ ((__unused__, section(".ctors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) (-1) };
static func_ptr __DTOR_LIST__[1]
  __attribute__ ((__unused__, section(".dtors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) (-1) };

static void call_ctors(void) {
  printf("Running constructors...\n");
  func_ptr *start = &__CTOR_LIST__[1];
  func_ptr *end = start;
  while (*end) ++end;
  int ctor_count = (end - start);
  for (int i = 0; i < ctor_count; ++i) {
    func_ptr ctor = start[ctor_count-i-1];
    ctor();
  }
}

static void call_dtors(void) {
  printf("Running destructors...\n");
  func_ptr *dtor = &__DTOR_LIST__[1];
  while (*dtor) (*dtor++)();
}

