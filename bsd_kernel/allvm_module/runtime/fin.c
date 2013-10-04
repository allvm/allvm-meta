//===-- fin.c -------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Finalize the ctors/dtors sections
// Link this in last to add null-terminating byte.
//
//===----------------------------------------------------------------------===//

typedef void (*func_ptr) (void);
static func_ptr __CTOR_END__[1]
  __attribute__ ((used, section(".ctors"))) = { 0 };
static func_ptr __DTOR_END__[1]
  __attribute__ ((used, section(".dtors"))) = { 0 };
