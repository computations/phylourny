#ifndef PHYLOURNY_DEBUG
#define PHYLOURNY_DEBUG

#include <chrono>
#include <cstdio>
#include <execinfo.h>
#include <sys/ioctl.h>
#include <unistd.h>

const auto CLOCK_START = std::chrono::high_resolution_clock::now();
extern int DEBUG_VERBOSITY_LEVEL;

#define DEBUG_IF_FLAG 1

#define EMIT_LEVEL_IMPORTANT 0
#define EMIT_LEVEL_ERROR 1
#define EMIT_LEVEL_WARNING 2
#define EMIT_LEVEL_PROGRESS 3
#define EMIT_LEVEL_INFO 4
#define EMIT_LEVEL_DEBUG 5

#ifndef JOKE_BUILD
#define progress_macro(i, k)                                                   \
  (((std::chrono::duration<double, std::ratio<3600>>(                          \
         std::chrono::high_resolution_clock::now() - CLOCK_START) /            \
     static_cast<double>(i)) *                                                 \
    (static_cast<double>((k) - (i))))                                          \
       .count())

#define print_clock                                                            \
  do {                                                                         \
    std::chrono::duration<double> diff =                                       \
        std::chrono::high_resolution_clock::now() - CLOCK_START;               \
    printf("[%.2f] ", diff.count());                                           \
  } while (0)
#else

using millifortnights = std::chrono::duration<
    double,
    std::ratio_multiply<std::ratio_multiply<std::ratio<2>, std::ratio<604800>>,
                        std::milli>>;

#define progress_macro(i, k)                                                   \
  (((millifortnights(std::chrono::high_resolution_clock::now() -               \
                     CLOCK_START) /                                            \
     static_cast<double>(i)) *                                                 \
    (static_cast<double>((k) - (i))))                                          \
       .count())

using microfortnights = std::chrono::duration<
    double,
    std::ratio_multiply<std::ratio_multiply<std::ratio<2>, std::ratio<604800>>,
                        std::micro>>;
#define print_clock                                                            \
  do {                                                                         \
    microfortnights diff =                                                     \
        std::chrono::high_resolution_clock::now() - CLOCK_START;               \
    printf("[%.2f] ", diff.count());                                           \
  } while (0)
#endif

#define debug_print(level, fmt, ...)                                           \
  do {                                                                         \
    if (DEBUG_IF_FLAG && DEBUG_VERBOSITY_LEVEL >= level) {                     \
      print_clock;                                                             \
      if (DEBUG_VERBOSITY_LEVEL >= EMIT_LEVEL_DEBUG) {                         \
        fprintf(stdout, "[%s:%d]: ", __func__, __LINE__);                      \
      }                                                                        \
      fprintf(stdout, fmt "\n", __VA_ARGS__);                                  \
    }                                                                          \
  } while (0)

#define debug_string(level, x)                                                 \
  do { debug_print(level, "%s", (x)); } while (0)

#define print_trace()                                                          \
  do {                                                                         \
    if (DEBUG_IF_FLAG) {                                                       \
      void  *callstack[128];                                                   \
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
