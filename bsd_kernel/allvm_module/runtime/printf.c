//===-- printf.c ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Redirect implementation for printf
//
//===----------------------------------------------------------------------===//

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>

#include <machine/stdarg.h>

int __real_printf(const char *, ...) __printflike(1, 2);
int	__real_vprintf(const char *, __va_list) __printflike(1, 0);


static const size_t FORMAT_LEN = 1 << 12;
static char buf[FORMAT_LEN];
static const char* prefix = "[ALLVM] ";

static const char* add_prefix(const char *fmt) {
  if ((strlcpy(buf, prefix, FORMAT_LEN) >= FORMAT_LEN) ||
      (strlcat(buf, fmt, FORMAT_LEN) >= FORMAT_LEN)) {
    __real_printf("[ALLVM] Format specifier overflow! :(\n");
    return 0;
  }
  return buf;
}

int __wrap_printf(const char * fmt, ...);
int __wrap_printf(const char * fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = -1;
  if ((fmt = add_prefix(fmt)))
    ret = __real_vprintf(fmt, ap);
  va_end(ap);
  return ret;
}

int __wrap_vprintf(const char* fmt, __va_list ap);
int __wrap_vprintf(const char* fmt, __va_list ap) {
  if ((fmt = add_prefix(fmt)))
    return __real_vprintf(fmt, ap);
  return -1;
}
