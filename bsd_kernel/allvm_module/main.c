/*
 * KLD Skeleton
 * Inspired by Andrew Reiter's Daemonnews article
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h> /* uprintf */
#include <sys/errno.h>
#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */

// CXX support
#include "icxxabi.h"

extern int testJIT();

/*
 * Load handler that deals with the loading and unloading of a KLD.
 */
static int jit_loader(struct module *m, int what, void *arg) {
  int err = 0;
  switch (what) {
  case MOD_LOAD: /* kldload */
    uprintf("ALLVM-JIT KLD loaded.\n");
    break;
  case MOD_UNLOAD:
    uprintf("ALLVM-JIT unloaded.\n");
    break;
  default:
    err = EOPNOTSUPP;
    break;
  }

  uprintf("Testing JIT...\n");
  uprintf("testJIT() returned: %d\n", testJIT());

  return (err);
}

/* Declare this module to the rest of the kernel */
static moduledata_t jit_mod = { "allvm-jit", jit_loader, NULL };

DECLARE_MODULE(allvm_jit, jit_mod, SI_SUB_KLD, SI_ORDER_ANY);
