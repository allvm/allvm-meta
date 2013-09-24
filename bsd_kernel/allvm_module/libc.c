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

#include "stubs.h"

void *__error = 0;

// TODO: Almost certainly terrible
void *__stderrp = 0;

void __assert(const char *, const char *, int, const char *);
void __assert(const char * function, const char * file, int line, const char *expression) {
  panic("Assertion failure in %s at %s:%d: \"%s\"\n", function, file, line, expression);
}

UNSUPPORTED(abort);
UNSUPPORTED(access);
UNSUPPORTED(alarm);
UNSUPPORTED(atoi);
UNSUPPORTED(clock_gettime);
UNSUPPORTED(close);
UNSUPPORTED(closedir);
UNSUPPORTED(dup2);
UNSUPPORTED(environ);
UNSUPPORTED(execv);
UNSUPPORTED(execve);
UNSUPPORTED(exit);
UNSUPPORTED(fabs);
UNSUPPORTED(fclose);
UNSUPPORTED(feof);
UNSUPPORTED(ferror);
UNSUPPORTED(fopen);
UNSUPPORTED(fork);
UNSUPPORTED(fputc);
UNSUPPORTED(fputs);
UNSUPPORTED(fread);
UNSUPPORTED(fstat);
UNSUPPORTED(ftruncate);
UNSUPPORTED(futimes);
UNSUPPORTED(fwrite);
UNSUPPORTED(getcwd);
UNSUPPORTED(getpagesize);
UNSUPPORTED(getpid);
UNSUPPORTED(getrlimit);
UNSUPPORTED(getrusage);
UNSUPPORTED(gettimeofday);
UNSUPPORTED(ioctl);
UNSUPPORTED(isalnum);
UNSUPPORTED(isalpha);
UNSUPPORTED(isatty);
UNSUPPORTED(isprint);
UNSUPPORTED(isspace);
UNSUPPORTED(kill);
UNSUPPORTED(link);
UNSUPPORTED(localtime_r);
UNSUPPORTED(lseek);
UNSUPPORTED(mkdir);
UNSUPPORTED(mmap);
UNSUPPORTED(mprotect);
UNSUPPORTED(munmap);
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
UNSUPPORTED(realloc);
UNSUPPORTED(realpath);
UNSUPPORTED(remove);
UNSUPPORTED(rename);
UNSUPPORTED(sbrk);
UNSUPPORTED(setrlimit);
UNSUPPORTED(sigaction);
UNSUPPORTED(sigemptyset);
UNSUPPORTED(sigfillset);
UNSUPPORTED(sigprocmask);
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
UNSUPPORTED(write);
UNSUPPORTED(writev);

UNSUPPORTED(__xuname);
UNSUPPORTED(_exit);
