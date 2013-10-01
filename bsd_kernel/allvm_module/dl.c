//===-- dl.c --------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// libdl stubs
//
//===----------------------------------------------------------------------===//

#include "stubs.h"

static void* const SELF_HANDLE = (void*)0xABC;

static const char* nullstr(const char *str) {
  return str ? "<null>" : str;
}

void* dlopen(const char*filename, int flag);
void* dlopen(const char*filename, int flag) {
  printf("[ALLVM] dlopen(filename=%s, flag=%d)\n", nullstr(filename), flag);
  if (!filename)
    return SELF_HANDLE;
  return 0;
}

int dlclose(void *handle);
int dlclose(void *handle) {
  printf("[ALLVM] dlclose(handle=%p)\n", handle);
  return 0;
}

char *dlerror(void);
char *dlerror(void) {
  printf("[ALLVM] dlerror()\n");
  return "dlerror not supported";
}

void *dlsym(void *handle, const char *symbol);
void *dlsym(void *handle, const char *symbol) {
  printf("[ALLVM] dlsym(handle=%p, symbol=%s)\n", handle, symbol);
  return 0;
}
