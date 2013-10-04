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

#include "icxxabi.h"
#include "begin.h"

#include "allvm.h"

static void cxx_init(void) {
    call_ctors();
}

static void cxx_fini(void) {
    __cxa_finalize(NULL);
    call_dtors();
}

static void* jit_handle = 0;

extern char bckernel_begin;
extern char bckernel_end;

// Handler invoked by kernel when module is loaded/unloaded
static int jit_loader(struct module *m, int what, void *arg) {
  int err = 0;
  switch (what) {
  case MOD_LOAD: /* kldload */
    printf("Initializing ALLVM-JIT KLD...\n");

    cxx_init();

    jit_handle = createJIT(&bckernel_begin, &bckernel_end, true);
    if (jit_handle) {
      printf("Successfully initialized ALLVM-JIT with kernel.bc\n");
      printf("ALLVM-JIT KLD loaded and initialized.\n");
    } else {
      printf("Failed to initialized ALLVM-JIT.\n");
      // TODO: Set 'err' appropriately
    }
    break;
  case MOD_UNLOAD:
    printf("ALLVM-JIT unloaded.\n");

    cxx_fini();

    if (jit_handle)
      destroyJIT(jit_handle);

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
