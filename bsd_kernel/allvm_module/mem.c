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

void *__real_malloc(unsigned long size, struct malloc_type *type, int flags) __malloc_like;
void *__real_realloc(void *addr, unsigned long size, struct malloc_type *type, int flags);
void __real_free(void *addr, struct malloc_type *type);

static inline void* alloc(unsigned long sz) {
  printf("[ALLVM] alloc(%lu)\n", sz);
  return __real_malloc(sz, allvm_mem, M_ZERO|M_NOWAIT);
}

static inline void dealloc(void *p) {
  printf("[ALLVM] dealloc(%p)\n", p);
  return __real_free(p, allvm_mem);
}

//===-- Define C operations in terms of alloc/dealloc ---------------------===//

// TODO: Force LLVM/etc to use *our* malloc, not that provided by kernel!

void *__wrap_malloc(size_t sz);
void *__wrap_malloc(size_t sz) {
  return alloc(sz);
}


void *calloc(size_t nmemb, size_t size);
void *calloc(size_t nmemb, size_t size) {
  return alloc(nmemb * size);
}

void *__wrap_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size) {
  // Deal with it :).
  return NULL;
}

void __wrap_free(void *ptr);
void __wrap_free(void *ptr) {
  dealloc(ptr);
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
