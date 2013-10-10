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
#include <sys/proc.h> // curthread

#include <machine/fpu.h>
#include <machine/specialreg.h>

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

#define start_emulating() load_cr0(rcr0() | CR0_TS)
#define stop_emulating()  clts()

static struct fpu_kern_ctx *fpu_ctx_save = 0;
static void fpu_init(void) {
  fpu_ctx_save = fpu_kern_alloc_ctx(FPU_KERN_NORMAL);
}
static int fpu_save(void) {
  int ret = !is_fpu_kern_thread(0) &&
    !fpu_kern_enter(curthread, fpu_ctx_save, FPU_KERN_NORMAL);
  stop_emulating();
  return ret;
}
static void fpu_restore(void) {
  fpu_kern_leave(curthread, fpu_ctx_save);
  start_emulating();
}
static void fpu_deinit(void) {
  fpu_kern_free_ctx(fpu_ctx_save);
}

static void* jit_handle = 0;

extern char bckernel_begin;
extern char bckernel_end;

// Handler invoked by kernel when module is loaded/unloaded
static int jit_loader(struct module *m, int what, void *arg) {

  int err = 0;
  switch (what) {
  case MOD_LOAD: /* kldload */
    fpu_init();
    if (!fpu_save()) {
      printf("Failed to save fpu context, aborting..\n");
      return 0;
    }

    printf("Initializing ALLVM-JIT KLD...\n");

    cxx_init();

    jit_handle = createJIT(&bckernel_begin, &bckernel_end, 1);
    if (jit_handle) {
      printf("Successfully initialized ALLVM-JIT with kernel.bc\n");
      printf("ALLVM-JIT KLD loaded and initialized.\n");
    } else {
      printf("Failed to initialized ALLVM-JIT.\n");
      // TODO: Set 'err' appropriately
    }
    fpu_restore();
    break;
  case MOD_UNLOAD:
    if (!fpu_save()) {
      printf("Failed to save fpu context, aborting..\n");
      return 0;
    }
    printf("ALLVM-JIT unloaded.\n");

    if (jit_handle)
      destroyJIT(jit_handle);

    cxx_fini();
    fpu_restore();
    fpu_deinit();

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
