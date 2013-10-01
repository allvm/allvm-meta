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
  void *ptr = __real_malloc(sz, allvm_mem, M_ZERO|M_NOWAIT);
  printf("alloc(%lu) = %p\n", sz, ptr);
  return ptr;
}

static inline void dealloc(void *p) {
  printf("dealloc(%p)\n", p);
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

  printf("mmap(addr=%p, len=%zu, prot=%d, flags=%d, fildes=%d, off=%zu)\n",
         addr, len, prot, flags, fildes, off);
  if ((len & (getpagesize() - 1)) != 0) {
    printf("  \\-> Invalid len to mmap: %zu\n", len);
    return MAP_FAILED;
  }
  if (!(flags & MAP_ANONYMOUS)) {
    printf(" \\-> Only anonymous mapping supported\n");
    return MAP_FAILED;
  }

  // XXX: current allocator already fills with zero
  return alloc(len);
}

int munmap(void *addr, size_t len);
int munmap(void *addr, size_t len) {
  printf("munmap(addr=%p, len=%zu)\n", addr, len);
  // TODO: Don't leak badly here :(
  return 0;
}


