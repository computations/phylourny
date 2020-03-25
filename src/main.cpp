#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_set>

#include "cli.hpp"
#include "debug.h"
#include "sampler.hpp"
#include "summary.hpp"
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
  //__VERBOSE__ = EMIT_LEVEL_PROGRESS1;
  auto start_time = std::chrono::high_resolution_clock::now();
  print_version();
  cli_options_t cli_options{argc, argv};
  size_t teams = 16;

  std::vector<match_t> matches;
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<> coin(0.0, 1.0);
  std::uniform_int_distribution team(0, 15);

  std::vector<double> params(teams);
  for (auto &p : params) {
    p = coin(gen);
  }

  for (size_t i = 0; i < 40; ++i) {
    size_t t1 = team(gen);
    size_t t2 = t1;
    while (t2 == t1) {
      t2 = team(gen);
    }
    double prob = params[t1] / (params[t1] + params[t2]);
    matches.push_back({t1, t2, coin(gen) < prob});
  }

  dataset_t ds{matches};
  sampler_t sampler{ds, tournament_factory(teams)};

  sampler.run_chain(10000, rd());
  auto summary = sampler.summary();

  std::ofstream outfile("summary.json");
  summary.write(outfile, 0, 1);

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
