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

typedef struct {
  size_t size;
  // TODO: Add magic bytes for sanity checking?
  char data[0];
} mem_hdr;

static const size_t hdr_offset = offsetof(mem_hdr, data);

static inline mem_hdr *get_header(void *p) {
  return (mem_hdr*)((char*)p - hdr_offset);
}

static inline size_t extract_size(void *p) {
  return get_header(p)->size;
}

static inline void* alloc(unsigned long sz) {
  size_t new_sz = sz + hdr_offset;
  mem_hdr *m = (mem_hdr *)__real_malloc(new_sz, allvm_mem, M_ZERO|M_NOWAIT);
  if (!m)
    return 0;
  m->size = sz;
  return &m->data[0];
}

static inline void dealloc(void *p) {
  if (p)
    __real_free(get_header(p), allvm_mem);
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
  // Allocate memory of requested size
  void *new_ptr = alloc(size);
  // Handle allocation failure
  if (!new_ptr)
    return 0;
  // realloc(NULL, size) is the same as malloc(size)
  if (!ptr)
    return new_ptr;

  // Otherwise, get the size of the old allocation
  // and copy its contents to the new one.
  size_t old_size = extract_size(ptr);
  memcpy(new_ptr, ptr, min(old_size, size));

  // Free old allocation and return new one
  dealloc(ptr);
  return new_ptr;
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

  if ((len & (getpagesize() - 1)) != 0) {
    printf("mmap(addr=%p, len=%zu, prot=%d, flags=%d, fildes=%d, off=%zu)\n",
           addr, len, prot, flags, fildes, off);
    printf("  \\-> Invalid len to mmap: %zu\n", len);
    return MAP_FAILED;
  }
  if (!(flags & MAP_ANONYMOUS)) {
    printf("mmap(addr=%p, len=%zu, prot=%d, flags=%d, fildes=%d, off=%zu)\n",
           addr, len, prot, flags, fildes, off);
    printf("  \\-> Only anonymous mapping supported\n");
    return MAP_FAILED;
  }

  //  (current allocator already fills with zero)
  void * ptr = alloc(len);
  // printf(" \\-> ptr: %p\n", ptr);
  return ptr;
}

int munmap(void *addr, size_t len);
int munmap(void *addr, size_t len) {
  // printf("munmap(addr=%p, len=%zu)\n", addr, len);

  if (!addr || extract_size(addr) != len) {
    printf("Unsupported use of munmap or bad pointer!\n");
    return -1;
  }
  dealloc(addr);
  return 0;
}


