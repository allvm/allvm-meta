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

#include "assert.h"
#include "debug.h"


//===-- Primary Allocation ------------------------------------------------===//

MALLOC_DECLARE(allvm_mem);
MALLOC_DEFINE(allvm_mem,"allvm-mem","Memory for ALLVM C/C++ kernel objects");

void *__real_malloc(unsigned long size, struct malloc_type *type, int flags) __malloc_like;
void *__real_realloc(void *addr, unsigned long size, struct malloc_type *type, int flags);
void __real_free(void *addr, struct malloc_type *type);

static inline void* alloc(unsigned long sz) {
  void *p = __real_malloc(sz, allvm_mem, M_ZERO|M_NOWAIT);
  assert(p && "Allocation failed");
  assert((((uintptr_t)p & 0x3) == 0) && "Insufficient pointer alignment");
  DEBUG(printf("alloc(sz=%zu) -> %p\n", sz, p));
  return p;
}

static inline void dealloc(void *p) {
  DEBUG(printf("dealloc(p=%p)\n", p));
  __real_free(p, allvm_mem);
}

static inline void *resizealloc(void *addr, unsigned long size) {
  void *p = __real_realloc(addr, size, allvm_mem, M_ZERO|M_NOWAIT);
  DEBUG(printf("realloc(addr=%p, size=%zu) -> %p\n", addr, size, p));
  return p;
}

//===-- Define C operations in terms of alloc/dealloc ---------------------===//

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
  return resizealloc(ptr, size);
}

void __wrap_free(void *ptr);
void __wrap_free(void *ptr) {
  dealloc(ptr);
}

int getpagesize(void);
int getpagesize(void) {
  // Obtained from 'sysctl hw.pagesize', TODO: Get this from kernel
  return 4096;
}

void *mmap(void *addr, size_t len, int prot, int flags,
           int fildes, off_t off);
void *mmap(void *addr, size_t len, int prot, int flags,
           int fildes, off_t off) {

  // From sys/mman.h:
  void *MAP_FAILED = (void*)-1;
  unsigned MAP_ANONYMOUS = 0x1000;
  DEBUG(
    printf("mmap(addr=%p, len=%zu, prot=%d, flags=%d, fildes=%d, off=%zu)\n",
         addr, len, prot, flags, fildes, off));

  if ((len & (getpagesize() - 1)) != 0) {
    DEBUG(printf("  \\-> Invalid len to mmap: %zu\n", len));
    return MAP_FAILED;
  }
  if (!(flags & MAP_ANONYMOUS)) {
    DEBUG(printf("  \\-> Only anonymous mapping supported\n"));
    return MAP_FAILED;
  }

  //  (current allocator already fills with zero)
  void * ptr = alloc(len);
  DEBUG(printf(" \\-> ptr: %p\n", ptr));
  return ptr;
}

int munmap(void *addr, size_t len);
int munmap(void *addr, size_t len) {
  DEBUG(printf("munmap(addr=%p, len=%zu)\n", addr, len));
  dealloc(addr);
  return 0;
}


