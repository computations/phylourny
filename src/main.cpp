#include <chrono>
#include <optional>
#include <string>
#include <unordered_set>
#include <iostream>
#include <json.hpp>

#include "cli.hpp"
#include "debug.h"
int __VERBOSE__ = EMIT_LEVEL_PROGRESS;

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define GIT_REV_STRING STRINGIFY(GIT_REV)
#define GIT_COMMIT_STRING STRINGIFY(GIT_COMMIT)
#define BUILD_DATE_STRING STRINGIFY(BUILD_DATE)

class team_t {
private:
  std::string label;
};

class tournament_node_t;

class tournament_node_ptr_t {
private:
  tournament_node_t *node;
  bool win;
};

class tournament_node_t {
private:
  std::optional<tournament_node_ptr_t> left_node;
  std::optional<tournament_node_ptr_t> right_node;
  std::unordered_set<team_t> teams;
};

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
typedef std::chrono::duration<double> duration_t;

void print_version() {
  debug_print(EMIT_LEVEL_IMPORTANT, "Version: %s", GIT_REV_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Commit: %s", GIT_COMMIT_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DATE_STRING);
}

void print_end_time(timepoint_t start_time, timepoint_t end_time) {
  duration_t duration = end_time - start_time;
  debug_print(EMIT_LEVEL_IMPORTANT, "Run Finished, time: %fs", duration.count());
}

int main(int argc, char **argv) {
  __VERBOSE__ = EMIT_LEVEL_PROGRESS;
  auto start_time = std::chrono::high_resolution_clock::now();
  cli_options_t cli_options{argc, argv};
  if(cli_options["bar"].initialized()){
    std::cout<<cli_options["bar"].value<bool>()<<std::endl;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
