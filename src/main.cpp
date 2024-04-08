#include <chrono>
#include <cstdint>
#include <csv.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <random>
#include <stdexcept>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "cli.hpp"
#include "debug.h"
#include "program_options.hpp"
int DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_PROGRESS;

#define TOSTR_(...) #__VA_ARGS__
#define STRING(...) TOSTR_(__VA_ARGS__)
#define STRINGIFY(s) STRING(s)
constexpr char GIT_REV_STRING[]     = STRINGIFY(GIT_REV);
constexpr char GIT_COMMIT_STRING[]  = STRINGIFY(GIT_COMMIT);
constexpr char BUILD_DATE_STRING[]  = STRINGIFY(BUILD_DATE);
constexpr char BUILD_DDATE_STRING[] = STRINGIFY(BUILD_DDATE);

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
using duration_t  = std::chrono::duration<double>;

static void print_version() {
  debug_string(EMIT_LEVEL_IMPORTANT, "Running Phylourny");
  debug_print(EMIT_LEVEL_IMPORTANT, "Version: %s", GIT_REV_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Commit: %s", GIT_COMMIT_STRING);
#ifdef JOKE_BUILD
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DDATE_STRING);
#else
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DATE_STRING);
#endif
}

static void print_run_info(const cli_options_t &cli_options) {
  debug_print(EMIT_LEVEL_IMPORTANT,
              "Using Seed: %lu",
              cli_options["seed"].value<uint64_t>());
#ifdef _OPENMP
#pragma omp parallel
  {
    if (omp_get_thread_num() == 0) {
      debug_print(EMIT_LEVEL_IMPORTANT,
                  "Running with %u threads",
                  omp_get_num_threads());
    }
  }
#endif
}

static void print_end_time(timepoint_t start_time, timepoint_t end_time) {
  duration_t duration = end_time - start_time;
#ifndef JOKE_BUILD
  debug_print(
      EMIT_LEVEL_IMPORTANT, "Run Finished, time: %fs", duration.count());
#else
  microfortnights mf_duration(duration);
  debug_print(EMIT_LEVEL_IMPORTANT,
              "Run Finished, time: %f microfortnights",
              mf_duration.count());
#endif
}

input_format_options_t
create_input_format_options(const cli_options_t &cli_options) {
  input_format_options_t ret;
  if (cli_options["matches"].initialized()) {
    ret.matches_filename = cli_options["matches"].value<std::string>();
  }
  if (cli_options["probs"].initialized()) {
    ret.probs_filename = cli_options["probs"].value<std::string>();
  }
  if (cli_options["odds"].initialized()) {
    ret.odds_filename = cli_options["odds"].value<std::string>();
  }
  if (cli_options["bestofs"].initialized()) {
    ret.bestofs_filename = cli_options["bestofs"].value<std::string>();
  }
  ret.dummy = cli_options["dummy"].value(false);
  return ret;
}

run_mode_t create_run_mode_type(const cli_options_t &cli_options) {
  run_mode_t ret;
  ret.dynamic    = cli_options["dynamic"].value(true);
  ret.single     = cli_options["single"].value(false);
  ret.simulation = cli_options["sim"].value(false);
  return ret;
}

simulation_mode_options_t
create_simulation_mode_options(const cli_options_t &cli_options) {
  simulation_mode_options_t sim_opts;
  sim_opts.samples = cli_options["sim-iters"].value(1'000'000lu);
  return sim_opts;
}

mcmc_options_t create_mcmc_options(const cli_options_t &cli_options) {
  mcmc_options_t mcmc_options;
  mcmc_options.model_type    = cli_options["poisson"].value(true)
                                   ? likelihood_model::poisson
                                   : likelihood_model::simple;
  mcmc_options.burnin        = cli_options["burnin"].value(0.1);
  mcmc_options.samples       = cli_options["samples"].value(100'000ul);
  mcmc_options.sample_matrix = cli_options["sample-matrix"].value(false);
  return mcmc_options;
}

std::vector<std::string> read_teams_file(const std::string &teams_filename) {
  std::ifstream teams_file(teams_filename);
  if (!teams_file) {
    throw std::runtime_error{"Could not read the teams file"};
  }
  std::vector<std::string> teams;

  std::string line;
  while (std::getline(teams_file, line)) { teams.push_back(line); }

  return teams;
}

program_options_t create_program_options(const cli_options_t &cli_options) {
  program_options_t prog_opts;
  prog_opts.input_formats      = create_input_format_options(cli_options);
  prog_opts.run_modes          = create_run_mode_type(cli_options);
  prog_opts.mcmc_options       = create_mcmc_options(cli_options);
  prog_opts.simulation_options = create_simulation_mode_options(cli_options);

  prog_opts.teams = read_teams_file(cli_options["teams"].value<std::string>());
  if (cli_options["seed"].initialized()) {
    prog_opts.seed = cli_options["seed"].value<uint64_t>();
  } else {
    std::random_device rd;
    prog_opts.seed = rd();
  }

  prog_opts.output_prefix = cli_options["prefix"].value<std::string>();

  return prog_opts;
}

void create_results_path(const std::string &results_prefix) {
  std::filesystem::path results_path = results_prefix;
  if (results_path.has_filename()) {
    results_path = results_path.parent_path();
  }

  auto results_status = std::filesystem::status(results_path);
  if (std::filesystem::exists(results_status) &&
      !std::filesystem::is_directory(results_status)) {
    throw std::runtime_error{"Results path exists and is not a directory"};
  }

  if (!results_path.empty()) {
    debug_print(EMIT_LEVEL_IMPORTANT,
                "Creating directory %s for prefix",
                results_path.c_str());

    std::filesystem::create_directories(results_path);
  }
}

auto main(int argc, char **argv) -> int {
  DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_PROGRESS;
  auto start_time       = std::chrono::high_resolution_clock::now();
  print_version();
  try {
    cli_options_t cli_options{argc, argv};

    if (cli_options["verbose"].value(false)) {
      DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_INFO;
      debug_string(EMIT_LEVEL_INFO, "Enabling info messages");
    }

    if (cli_options["debug"].value<bool>(false)) {
      DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_DEBUG;
    }

    auto program_options = create_program_options(cli_options);

    std::vector<std::string> teams;

    {
      std::ifstream teams_file(cli_options["teams"].value<std::string>());
      for (std::string tmp; std::getline(teams_file, tmp);) {
        if (tmp.empty()) { continue; }
        teams.push_back(tmp);
      }
    }

    /*
     * This janky if statement is so that we don't read from the random device
     * if we don't need to. I know, it's not really that important, but it
     * matters to me lol.
     */
    if (!cli_options["seed"].initialized()) {
      std::random_device rd;
      cli_options["seed"].set_if_unset<uint64_t>(rd());
      assert_string(cli_options["seed"].initialized(),
                    "CLI Option seed should be initilaized here");
    }

    print_run_info(cli_options);

    create_results_path(program_options.output_prefix);

    run(program_options);

    auto end_time = std::chrono::high_resolution_clock::now();
    print_end_time(start_time, end_time);

  } catch (cli_option_help &e) {
    (void)e;
    std::cout << cli_options_t::help();
    return 1;
  } catch (cli_option_exception &e) {
    std::cout << e.what() << std::endl;
    std::cout << cli_options_t::help();
    return 1;
  }

  return 0;
}
