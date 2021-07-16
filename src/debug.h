#ifndef __PHYLOURNY_DEBUG__
#define __PHYLOURNY_DEBUG__

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

const clock_t CLOCK_START = clock();
extern bool   __PROGRESS_BAR_FLAG__;
extern int    __VERBOSE__;

#define DEBUG_IF_FLAG 1

#ifdef RD_DEBUG
#define RD_DEBUG_ASSERT_FLAG 1
#else
#define RD_DEBUG_ASSERT_FLAG 0
#endif

#define EMIT_LEVEL_IMPORTANT 0
#define EMIT_LEVEL_ERROR 1
#define EMIT_LEVEL_WARNING 2
#define EMIT_LEVEL_PROGRESS 3
#define EMIT_LEVEL_INFO 4
#define EMIT_LEVEL_DEBUG 5

#define print_clock                                                            \
  do {                                                                         \
    fprintf(                                                                   \
        stdout, "[%.2f] ", ((double)clock() - CLOCK_START) / CLOCKS_PER_SEC);  \
  } while (0)

#define debug_print(level, fmt, ...)                                           \
  do {                                                                         \
    if (DEBUG_IF_FLAG && __VERBOSE__ >= level) {                               \
      print_clock;                                                             \
      if (__VERBOSE__ >= EMIT_LEVEL_DEBUG) {                                   \
        fprintf(stdout, "[%s:%d]: ", __func__, __LINE__);                      \
      }                                                                        \
      fprintf(stdout, fmt "\n", __VA_ARGS__);                                  \
    }                                                                          \
  } while (0)

#define debug_string(level, x)                                                 \
  do { debug_print(level, "%s", x); } while (0)

#define print_trace()                                                          \
  do {                                                                         \
    if (DEBUG_IF_FLAG) {                                                       \
      void * callstack[128];                                                   \
      int    frames     = backtrace(callstack, 128);                           \
      char **bt_symbols = backtrace_symbols(callstack, frames);                \
      print_clock;                                                             \
      fprintf(                                                                 \
          stderr, "BACKTRACE AT %s:%d:%s():\n", __FILE__, __LINE__, __func__); \
      for (int i = 0; i < frames; ++i) {                                       \
        print_clock;                                                           \
        fprintf(stderr, "%s\n", bt_symbols[i]);                                \
      }                                                                        \
    }                                                                          \
  } while (0)

#define assert_string(cond, comment)                                           \
  do {                                                                         \
    if (DEBUG_IF_FLAG) {                                                       \
      {                                                                        \
        if (!(cond)) {                                                         \
          print_clock;                                                         \
          fprintf(                                                             \
              stderr,                                                          \
              "assertion \"%s\" failed: file: %s, line: %d, comment: %s\n",    \
              #cond,                                                           \
              __FILE__,                                                        \
              __LINE__,                                                        \
              comment);                                                        \
          abort();                                                             \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

#endif
