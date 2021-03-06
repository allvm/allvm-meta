//===-- libc.c ------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation of subset of libc needed for ALLVM-JIT
//
//===----------------------------------------------------------------------===//

#include "common.h"
#include "debug.h"

static int errno = 0;
int *__error(void);
int *__error(void) {
  return &errno;
}

// TODO: Almost certainly terrible
void *__stdinp = (void*)0xAAAA0000;
void *__stderrp = (void*)0xBBBB0000;
void *__stdoutp = (void*)0xCCCC0000;

void __assert(const char *, const char *, int, const char *);
void __assert(const char * function, const char * file, int line, const char *expression) {
  // Ensure we log that assertion failed, regardless of validity of arguments.
  printf("Assertion failure!\n");
  // Log the error explicitly so serial console gets it
  printf("Assertion failure in %s at %s:%d: \"%s\"\n", nullstr(function),
        nullstr(file), line, nullstr(expression));
  // panic, probably duplicating message.
  panic("Assertion failure in %s at %s:%d: \"%s\"\n", nullstr(function),
        nullstr(file), line, nullstr(expression));
}

int isalpha(int c);
int isalpha(int c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z');
}

int isdigit(int c);
int isdigit(int c) {
  return '0' <= c && c <= '9';
}

int isxdigit(int c);
int isxdigit(int c) {
  return isdigit(c) ||
    ('a' <= c && c <= 'f') ||
    ('A' <= c && c <= 'F');
}

int isalnum(int c);
int isalnum(int c) {
  return isalpha(c) || isdigit(c);
}

int isspace(int c);
int isspace(int c) {
  return c == ' '  || c == '\f' ||
         c == '\n' || c == '\r' ||
         c == '\t' || c == '\v';
}

int isprint(int c);
int isprint(int c) {
  // For the standard ASCII character set (used by the "C" locale), printing
  // characters are all with an ASCII code greater than 0x1f (US), except 0x7f
  // (DEL).
  // (from http://www.cplusplus.com/reference/cctype/isprint/)

  return (c != 0x7f) && (c >= 0x1f) && (c <= 0xff);
}

char *__wrap_getenv(const char* key);
char *__wrap_getenv(const char* key) {
  DEBUG(printf("getenv(key=%s)\n", nullstr(key)));
  return 0;
}

int gettimeofday(void *tp, void *tzp);
int gettimeofday(void *tp, void *tzp) {
  TRACE();
  return -1;
}

char *getcwd(char *buf, size_t size);
char *getcwd(char *buf, size_t size) {
  TRACE();
  return 0;
}

void perror(const char *s);
void perror(const char *s) {
  DEBUG(printf("perror(s=%s)\n", nullstr(s)));
}

off_t lseek(int fd, off_t offset, int whence);
off_t lseek(int fd, off_t offset, int whence) {
  DEBUG(printf("lseek(fd=%d, offset=%zd, whence=%d)\n", fd, offset, whence));
  return -1;
}

int fstat(int fd, void* buf);
int fstat(int fd, void* buf) {
  DEBUG(printf("fstat(fd=%d, ...)\n", fd));
  return -1;
}

void abort(void);
void abort(void) {
  TRACE();
  panic("[ALLVM] ABORT CALLED!\n");
}

int close(int fd);
int close(int fd) {
  DEBUG(printf("close(fd=%d) called\n", fd));
  if (fd >= 0 && fd <= 2)
    return 0;
  return -1;
}

UNSUPPORTED(__xuname);
UNSUPPORTED(_exit);
UNSUPPORTED(access);
UNSUPPORTED(alarm);
UNSUPPORTED(clearerr);
UNSUPPORTED(clock_gettime);
UNSUPPORTED(closedir);
UNSUPPORTED(dup2);
UNSUPPORTED(environ);
UNSUPPORTED(execv);
UNSUPPORTED(execve);
UNSUPPORTED(exit);
UNSUPPORTED(fabs);
UNSUPPORTED(fclose);
UNSUPPORTED(fcntl);
UNSUPPORTED(feof);
UNSUPPORTED(ferror);
UNSUPPORTED(fflush);
UNSUPPORTED(fileno);
UNSUPPORTED(fopen);
UNSUPPORTED(fork);
UNSUPPORTED(fprintf);
UNSUPPORTED(fputc);
UNSUPPORTED(fread);
UNSUPPORTED(fseek);
UNSUPPORTED(ftell);
UNSUPPORTED(ftruncate);
UNSUPPORTED(futimes);
UNSUPPORTED(fwrite);
UNSUPPORTED(getpid);
UNSUPPORTED(getrlimit);
UNSUPPORTED(getrusage);
UNSUPPORTED(ioctl);
UNSUPPORTED(isatty);
UNSUPPORTED(kill);
UNSUPPORTED(link);
UNSUPPORTED(localtime_r);
UNSUPPORTED(mkdir);
UNSUPPORTED(mprotect);
UNSUPPORTED(open);
UNSUPPORTED(opendir);
UNSUPPORTED(posix_spawn);
UNSUPPORTED(posix_spawn_file_actions_adddup2);
UNSUPPORTED(posix_spawn_file_actions_addopen);
UNSUPPORTED(posix_spawn_file_actions_destroy);
UNSUPPORTED(posix_spawn_file_actions_init);
UNSUPPORTED(pread);
UNSUPPORTED(raise);
UNSUPPORTED(read);
UNSUPPORTED(readdir);
UNSUPPORTED(realpath);
UNSUPPORTED(remove);
UNSUPPORTED(rename);
UNSUPPORTED(sbrk);
UNSUPPORTED(setrlimit);
UNSUPPORTED(stat);
UNSUPPORTED(strerror_r);
UNSUPPORTED(strftime);
UNSUPPORTED(strsignal);
UNSUPPORTED(strtod);
UNSUPPORTED(symlink);
UNSUPPORTED(sysconf);
UNSUPPORTED(tolower);
UNSUPPORTED(toupper);
UNSUPPORTED(truncate);
UNSUPPORTED(unlink);
UNSUPPORTED(wait);
UNSUPPORTED(waitpid);
UNSUPPORTED(writev);
