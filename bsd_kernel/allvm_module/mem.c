//===-- mem.c -------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Support for C/C++ memory allocation in kernel
//
//===----------------------------------------------------------------------===//

#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */
#include <sys/malloc.h>
#include <sys/systm.h>

//===-- Primary Allocation ------------------------------------------------===//

MALLOC_DECLARE(allvm_mem);
MALLOC_DEFINE(allvm_mem,"allvm-mem","Memory for ALLVM C/C++ kernel objects");

static inline void* alloc(unsigned long sz) {
  printf("[ALLVM] alloc(%lu)\n", sz);
  return malloc(sz, allvm_cpp_mem, M_ZERO|M_NOWAIT);
}

static inline void dealloc(void *p) {
  printf("[ALLVM] dealloc(%p)\n", p);
  return free(p, allvm_cpp_mem);
}

//===-- Define C operations in terms of alloc/dealloc ---------------------===//

// TODO: Force LLVM/etc to use *our* malloc, not that provided by kernel!


void *calloc(size_t nmemb, size_t size) {
  return alloc(nmemb * size);
}

void *realloc(void *ptr, size_t size) {
  // Deal with it :).
  return NULL;
}


//===-- Define C++ operations in terms of alloc/dealloc -------------------===//

// operator new[](unsigned long)
void* _Znam(unsigned long sz);
void* _Znam(unsigned long sz) {
  return alloc(sz);
}
// operator new(unsigned long)
void *_Znwm(unsigned long sz);
void *_Znwm(unsigned long sz) {
  return alloc(sz);
}
// operator new(unsigned long, std::nothrow_t const&)
void *_ZnwmRKSt9nothrow_t(unsigned long sz, int unused);
void *_ZnwmRKSt9nothrow_t(unsigned long sz, int unused) {
  return alloc(sz);
}

// operator delete[](void*)
void _ZdaPv(void *p);
void _ZdaPv(void *p) {
  dealloc(p);
}
// operator delete(void*)
void _ZdlPv(void *p);
void _ZdlPv(void *p) {
  dealloc(p);
}
// operator delete(void*, std::nothrow_t const&)
void _ZdlPvRKSt9nothrow_t(void *p, int unused);
void _ZdlPvRKSt9nothrow_t(void *p, int unused) {
  dealloc(p);
}
