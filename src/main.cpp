#include <algorithm>
#include <chrono>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

#include "cli.hpp"
#include "debug.h"
#include "tournament.hpp"
int __VERBOSE__ = EMIT_LEVEL_PROGRESS;

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define GIT_REV_STRING STRINGIFY(GIT_REV)
#define GIT_COMMIT_STRING STRINGIFY(GIT_COMMIT)
#define BUILD_DATE_STRING STRINGIFY(BUILD_DATE)

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
typedef std::chrono::duration<double> duration_t;

void print_version() {
  debug_string(EMIT_LEVEL_IMPORTANT, "Running Phylourny");
  debug_print(EMIT_LEVEL_IMPORTANT, "Version: %s", GIT_REV_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Commit: %s", GIT_COMMIT_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DATE_STRING);
}

void print_end_time(timepoint_t start_time, timepoint_t end_time) {
  duration_t duration = end_time - start_time;
  debug_print(EMIT_LEVEL_IMPORTANT, "Run Finished, time: %fs",
              duration.count());
}

int main(int argc, char **argv) {
  __VERBOSE__ = EMIT_LEVEL_PROGRESS;
  auto start_time = std::chrono::high_resolution_clock::now();
  print_version();
  cli_options_t cli_options{argc, argv};

  std::unique_ptr<tournament_node_t> n1{new tournament_node_t};
  std::unique_ptr<tournament_node_t> n2{new tournament_node_t};

  std::unique_ptr<tournament_node_t> p1{
      new tournament_node_t{std::move(n1), std::move(n2)}};

  std::unique_ptr<tournament_node_t> n3{new tournament_node_t};
  std::unique_ptr<tournament_node_t> n4{new tournament_node_t};

  std::unique_ptr<tournament_node_t> p2{
      new tournament_node_t{std::move(n3), std::move(n4)}};

  tournament_t t{tournament_node_t{std::move(p1), std::move(p2)}};
  std::vector<std::vector<double>> win_probs{
      {0.00, 0.75, 0.75, 0.75},
      {0.25, 0.00, 0.75, 0.75},
      {0.25, 0.25, 0.00, 0.75},
      {0.25, 0.25, 0.25, 0.00},
  };

  debug_print(EMIT_LEVEL_IMPORTANT, "win_probs:\n%s",
              to_string(win_probs).c_str());

  t.relabel_indicies();
  t.reset_win_probs(win_probs);

  auto wp = t.eval();
  debug_print(EMIT_LEVEL_IMPORTANT, "eval(): %s", to_string(wp).c_str());
  double sum = 0.0;
  for (auto f : wp) {
    sum += f;
  }

  debug_print(EMIT_LEVEL_IMPORTANT, "sum: %f", sum);

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
