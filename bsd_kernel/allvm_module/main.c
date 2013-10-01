//===-- main.c ------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// ALLVM-JIT primary kernel module interface
//
//===----------------------------------------------------------------------===//

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h> /* uprintf */
#include <sys/errno.h>
#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */

// CXX support
#include "icxxabi.h"

extern int testJIT(char go);

typedef void (*func_ptr) (void);
static func_ptr __CTOR_LIST__[1]
  __attribute__ ((__unused__, section(".ctors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) (-1) };
static func_ptr __CTOR_END__[1]
  __attribute__((section(".ctors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) 0 };

/*
 * Load handler that deals with the loading and unloading of a KLD.
 */
static int jit_loader(struct module *m, int what, void *arg) {
  int err = 0;
  switch (what) {
  case MOD_LOAD: /* kldload */
    printf("ALLVM-JIT KLD loaded.\n");

    printf("Testing JIT...\n");
    printf("CTOR_LIST: %p\n", &__CTOR_LIST__);
    printf("CTOR_END: %p\n", &__CTOR_END__);
    printf("testJIT() returned: %d\n", testJIT(false));

    break;
  case MOD_UNLOAD:
    printf("ALLVM-JIT unloaded.\n");
    break;
  default:
    err = EOPNOTSUPP;
    break;
  }

  return (err);
}

/* Declare this module to the rest of the kernel */
static moduledata_t jit_mod = { "allvm-jit", jit_loader, NULL };

DECLARE_MODULE(allvm_jit, jit_mod, SI_SUB_KLD, SI_ORDER_ANY);
