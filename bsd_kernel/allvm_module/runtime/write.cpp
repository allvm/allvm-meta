//===-- write.cpp ---------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Support for 'write()' to stdout/stderr only, line-buffered
//
//===----------------------------------------------------------------------===//

#include "common.h"

#include <algorithm>
#include <cstring>
#include <vector>

static std::vector<char> stdout_buffer;
static std::vector<char> stderr_buffer;

extern "C" int __real_printf(const char *, ...) __printflike(1, 2);

static void print_char(char c) {
  __real_printf("%c", c);
}

extern "C" ssize_t write(int fd, const void *buf, size_t count);

static void __attribute__((destructor)) flush() {
  const char * stdout_msg = "(Flushing stdout...)\n";
  const char * stderr_msg = "(Flushing stderr...)\n";
  write(1,  stdout_msg, strlen(stdout_msg));
  write(2,  stderr_msg, strlen(stderr_msg));
}

template <typename It>
void print(It start, It end) {
  if (start == end) return;

  // If no null bytes, print as single string
  if (std::find(start, end, 0) == end) {
    std::vector<char> temp(start, end);
    temp.push_back(0);
    __real_printf("%s", &temp[0]);
  } else {
    // Otherwise just print each char
    std::for_each(start, end, print_char);
  }
}

ssize_t write(int fd, const void *buf, size_t count) {
  // Only support write() to stdout or stderr... (which are treated the same)
  if (fd != 1 && fd != 2)
    return -1;

  std::vector<char> & buffer = (fd == 1) ? stdout_buffer : stderr_buffer;
  const char *prefix = (fd == 1) ? "[ALLVM-STDOUT] " : "[ALLVM-STDERR] ";

  // Insert into buffer
  const char *start = (const char*)buf, *end = start + count;
  while (start != end) {
    const char * newline = std::find(start, end, '\n');
    if (newline != end) {
      // Write prefix
      __real_printf(prefix);

      // Print buffered contents
      print(buffer.begin(), buffer.end());
      buffer.clear();

      // And portion of line until newline
      print(start, newline);

      // And end line
      __real_printf("\n");
      start = ++newline;
    } else {
      // Buffer rest of line
      buffer.insert(buffer.end(), start, end);
      start = end;
    }
  }

  return count;
}

