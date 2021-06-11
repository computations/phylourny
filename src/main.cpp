#include <algorithm>
#include <chrono>
#include <cstdint>
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

std::vector<match_t> make_dummy_data(size_t team_count) {
  std::vector<match_t> matches;
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_real_distribution<> coin(0.0, 1.0);
  std::uniform_int_distribution team(0, static_cast<int>(team_count) - 1);

  std::vector<double> params(team_count);
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

  return matches;
}

int main(int argc, char **argv) {
  __VERBOSE__ = EMIT_LEVEL_PROGRESS;
  auto start_time = std::chrono::high_resolution_clock::now();
  print_version();
  cli_options_t cli_options{argc, argv};

  if (cli_options["debug"].value<bool>(false)) {
    __VERBOSE__ = EMIT_LEVEL_DEBUG;
  }

  std::vector<std::string> teams;

  {
    std::ifstream teams_file(cli_options["teams"].value<std::string>());
    std::string tmp;
    while (teams_file >> tmp) {
      teams.push_back(tmp);
    }
  }

  std::vector<match_t> matches;
  if (cli_options["dummy"].value<bool>(false)) {
    debug_string(EMIT_LEVEL_IMPORTANT, "Making dummy data");
    matches = make_dummy_data(teams.size());
  }

  uint64_t seed;
  if (cli_options["seed"].initialized()) {
    seed = cli_options["seed"].value<uint64_t>();
  } else {
    std::random_device rd;
    seed = rd();
  }

  dataset_t ds{matches};
  sampler_t sampler{ds, tournament_factory(teams)};

  sampler.run_chain(10000, seed);
  auto summary = sampler.summary();

  std::ofstream outfile(cli_options["output"].value<std::string>());
  summary.write_samples(outfile, 0, 1);

  std::ofstream mpp_outfile("test_mpp");
  summary.write_mpp(mpp_outfile);

  std::ofstream mmpp_outfile("test_mmpp");
  summary.write_mmpp(mmpp_outfile);

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
