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

#include "common.h"
#include "debug.h"

static void* const SELF_HANDLE = (void*)0xABC;

void* dlopen(const char*filename, int flag);
void* dlopen(const char*filename, int flag) {
  DEBUG(printf("dlopen(filename=%s, flag=%d)\n", nullstr(filename), flag));
  if (!filename)
    return SELF_HANDLE;
  return 0;
}

int dlclose(void *handle);
int dlclose(void *handle) {
  DEBUG(printf("dlclose(handle=%p)\n", handle));
  return 0;
}

char *dlerror(void);
char *dlerror(void) {
  DEBUG(printf("dlerror()\n"));
  return "dlerror not supported";
}

void *dlsym(void *handle, const char *symbol);
void *dlsym(void *handle, const char *symbol) {
  DEBUG(printf("dlsym(handle=%p, symbol=%s)\n", handle, symbol));
  return 0;
}
