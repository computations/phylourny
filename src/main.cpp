#include <algorithm>
#include <chrono>
#include <cstdint>
#include <csv.h>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "cli.hpp"
#include "dataset.hpp"
#include "debug.h"
#include "sampler.hpp"
#include "single_node.hpp"
#include "summary.hpp"
#include "tournament.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"
int __VERBOSE__ = EMIT_LEVEL_PROGRESS;

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define GIT_REV_STRING STRINGIFY(GIT_REV)
#define GIT_COMMIT_STRING STRINGIFY(GIT_COMMIT)
#define BUILD_DATE_STRING STRINGIFY(BUILD_DATE)

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
typedef std::chrono::duration<double>                               duration_t;

typedef std::unordered_map<std::string, size_t> team_name_map_t;

void print_version() {
  debug_string(EMIT_LEVEL_IMPORTANT, "Running Phylourny");
  debug_print(EMIT_LEVEL_IMPORTANT, "Version: %s", GIT_REV_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Commit: %s", GIT_COMMIT_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DATE_STRING);
}

void print_end_time(timepoint_t start_time, timepoint_t end_time) {
  duration_t duration = end_time - start_time;
  debug_print(
      EMIT_LEVEL_IMPORTANT, "Run Finished, time: %fs", duration.count());
}

std::vector<match_t> make_dummy_data(size_t team_count) {
  std::vector<match_t>             matches;
  std::random_device               rd;
  std::mt19937_64                  gen(rd());
  std::uniform_real_distribution<> coin(0.0, 1.0);
  std::uniform_int_distribution    team(0, static_cast<int>(team_count) - 1);

  std::vector<double> params(team_count);
  for (auto &p : params) { p = coin(gen); }

  for (size_t i = 0; i < 40; ++i) {
    size_t t1 = team(gen);
    size_t t2 = t1;
    while (t2 == t1) { t2 = team(gen); }
    double prob = params[t1] / (params[t1] + params[t2]);
    matches.push_back(
        {t1,
         t2,
         coin(gen) < prob ? match_winner_t::left : match_winner_t::right});
  }

  return matches;
}

team_name_map_t create_name_map(std::vector<std::string> team_names) {
  team_name_map_t name_map;

  for (size_t i = 0; i < team_names.size(); i++) {
    name_map[team_names[i]] = i;
  }

  return name_map;
}

matrix_t parse_odds_file(const std::string &    odds_filename,
                         const team_name_map_t &name_map) {

  matrix_t odds;

  odds.resize(name_map.size());
  for (auto &&r : odds) { r.resize(name_map.size()); }

  io::CSVReader<4> odds_file(odds_filename);
  odds_file.read_header(
      io::ignore_extra_column, "team1", "team2", "odds1", "odds2");

  std::string team1, team2;
  double      odds1, odds2;
  while (odds_file.read_row(team1, team2, odds1, odds2)) {
    size_t index1 = name_map.at(team1);
    size_t index2 = name_map.at(team2);

    double prob = odds1 / (odds1 + odds2);

    odds[index1][index2] = prob;
    odds[index2][index1] = 1 - prob;
  }

  return odds;
}

std::vector<match_t> parse_match_file(const std::string &    match_filename,
                                      const team_name_map_t &name_map) {
  std::vector<match_t> match_history;

  io::CSVReader<3> match_file(match_filename);
  match_file.read_header(io::ignore_extra_column, "team1", "team2", "winner");
  std::string team1, team2, winner;
  while (match_file.read_row(team1, team2, winner)) {
    size_t index1       = name_map.at(team1);
    size_t index2       = name_map.at(team2);
    size_t winner_index = name_map.at(winner);
    match_history.push_back({index1,
                             index2,
                             winner_index == index1 ? match_winner_t::right
                                                    : match_winner_t::left});
  }

  return match_history;
}

matrix_t parse_prob_files(const std::string &    probs_filename,
                          const team_name_map_t &name_map) {

  matrix_t win_probs;
  win_probs.resize(name_map.size());

  for (auto &&v : win_probs) { v.resize(name_map.size()); }

  io::CSVReader<3> probs_file(probs_filename);
  probs_file.read_header(
      io::ignore_extra_column, "team1", "team2", "prob-win-team1");
  std::string team1, team2;
  double      win_prob;
  while (probs_file.read_row(team1, team2, win_prob)) {
    size_t team1_index                  = name_map.at(team1);
    size_t team2_index                  = name_map.at(team2);
    win_probs[team1_index][team2_index] = win_prob;
    win_probs[team2_index][team1_index] = 1.0 - win_prob;
  }

  return win_probs;
}

int main(int argc, char **argv) {
  __VERBOSE__     = EMIT_LEVEL_PROGRESS;
  auto start_time = std::chrono::high_resolution_clock::now();
  print_version();
  try {
    cli_options_t cli_options{argc, argv};

    if (cli_options["debug"].value<bool>(false)) {
      __VERBOSE__ = EMIT_LEVEL_DEBUG;
    }

    std::vector<std::string> teams;

    {
      std::ifstream teams_file(cli_options["teams"].value<std::string>());
      for (std::string tmp; std::getline(teams_file, tmp);) {
        teams.push_back(tmp);
      }
    }

    auto team_name_map = create_name_map(teams);

    std::vector<match_t> matches;
    if (cli_options["matches"].initialized()) {
      matches = parse_match_file(cli_options["matches"].value<std::string>(),
                                 team_name_map);
    } else if (cli_options["dummy"].value<bool>(false)) {
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

    std::string output_prefix = cli_options["prefix"].value<std::string>();

    if (cli_options["matches"].initialized() ||
        cli_options["dummy"].initialized()) {

      if (cli_options["single"].value<bool>(false)) {
        /* This is an insane set of parameters for single mode. I suspect that
         * it will never complete for the team size
         */
        dataset_t                ds{matches};
        sampler_t<single_node_t> sampler{ds, tournament_factory_single(teams)};

        debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler");
        sampler.run_chain(1000, seed);
        auto summary = sampler.summary();

        std::ofstream outfile(output_prefix + ".samples.json");
        summary.write_samples(outfile, 0, 1);

        std::ofstream mlp_outfile(output_prefix + ".mlp.json");
        summary.write_mlp(mlp_outfile, 10);

        std::ofstream mmpp_outfile(output_prefix + ".mmpp.json");
        summary.write_mmpp(mmpp_outfile, 10);

      } else {
        dataset_t                    ds{matches};
        sampler_t<tournament_node_t> sampler{ds, tournament_factory(teams)};

        debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler");
        sampler.run_chain(10000000, seed);
        auto summary = sampler.summary();

        std::ofstream outfile(output_prefix + ".samples.json");
        summary.write_samples(outfile, 0, 1);

        std::ofstream mlp_outfile(output_prefix + ".mlp.json");
        summary.write_mlp(mlp_outfile, 1000000);

        std::ofstream mmpp_outfile(output_prefix + ".mmpp.json");
        summary.write_mmpp(mmpp_outfile, 1000000);
      }
    }

    if (cli_options["odds"].initialized()) {
      matrix_t odds;
      odds = parse_odds_file(cli_options["odds"].value<std::string>(),
                             team_name_map);
      std::ofstream odds_outfile(output_prefix + ".odds.json");
      if (cli_options["single"].value<bool>(false)) {
        auto t = tournament_factory_single(teams);
        t.reset_win_probs(odds);
        auto wp = t.eval();
        odds_outfile << to_json(wp) << std::endl;
      } else {
        auto t = tournament_factory(teams);
        t.reset_win_probs(odds);
        auto wp = t.eval();
        odds_outfile << to_json(wp) << std::endl;
      }
    }

    if (cli_options["probs"].initialized()) {
      matrix_t probs = parse_prob_files(
          cli_options["probs"].value<std::string>(), team_name_map);
      std::ofstream probs_outfile(output_prefix + ".probs.json");
      if (cli_options["single"].value<bool>(false)) {
        auto t = tournament_factory_single(teams);
        t.reset_win_probs(probs);
        auto wp = t.eval();
        probs_outfile << to_json(wp) << std::endl;
      } else {
        auto t = tournament_factory(teams);
        t.reset_win_probs(probs);
        auto wp = t.eval();
        probs_outfile << to_json(wp) << std::endl;
      }
    }
  } catch (cli_option_help &e) { return 1; } catch (cli_option_exception &e) {
    std::cout << e.what() << std::endl;
    std::cout << cli_options_t::help();
    return 1;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
