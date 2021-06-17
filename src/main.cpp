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

typedef std::unordered_map<std::string, size_t> team_name_map_t;

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
    matches.push_back(
        {t1, t2,
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

matrix_t parse_odds_file(const std::string &odds_filename,
                         const team_name_map_t &name_map) {

  matrix_t odds;

  odds.resize(name_map.size());
  for (auto &&r : odds) {
    r.resize(name_map.size());
  }

  io::CSVReader<4> odds_file(odds_filename);
  odds_file.read_header(io::ignore_extra_column, "team1", "team2", "odds1",
                        "odds2");

  std::string team1, team2;
  double odds1, odds2;
  while (odds_file.read_row(team1, team2, odds1, odds2)) {
    size_t index1 = name_map.at(team1);
    size_t index2 = name_map.at(team2);

    double prob = odds1 / (odds1 + odds2);

    odds[index1][index2] = prob;
    odds[index2][index1] = 1 - prob;
  }

  return odds;
}

std::vector<match_t> parse_match_file(const std::string &match_filename,
                                      const team_name_map_t &name_map) {
  std::vector<match_t> match_history;

  io::CSVReader<3> match_file(match_filename);
  match_file.read_header(io::ignore_extra_column, "team1", "team2", "winner");
  std::string team1, team2, winner;
  while (match_file.read_row(team1, team2, winner)) {
    size_t index1 = name_map.at(team1);
    size_t index2 = name_map.at(team2);
    size_t winner_index = name_map.at(winner);
    match_history.push_back({index1, index2,
                             winner_index == index1 ? match_winner_t::right
                                                    : match_winner_t::left});
  }

  return match_history;
}

int main(int argc, char **argv) {
  __VERBOSE__ = EMIT_LEVEL_PROGRESS;
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
      std::string tmp;
      while (teams_file >> tmp) {
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

      dataset_t ds{matches};
      sampler_t sampler{ds, tournament_factory(teams)};

      debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler");
      sampler.run_chain(10000000, seed);
      auto summary = sampler.summary();

      std::ofstream outfile(output_prefix + ".samples.json");
      summary.write_samples(outfile, 0, 1);

      std::ofstream mpp_outfile(output_prefix + ".mpp.json");
      summary.write_mpp(mpp_outfile);

      std::ofstream mmpp_outfile(output_prefix + ".mmpp.json");
      summary.write_mmpp(mmpp_outfile);
    }

    if (cli_options["odds"].initialized()) {
      matrix_t odds;
      odds = parse_odds_file(cli_options["odds"].value<std::string>(),
                             team_name_map);
      auto t = tournament_factory(teams);
      t.reset_win_probs(odds);
      auto wp = t.eval();
      std::ofstream odds_outfile(output_prefix + ".odds.json");
      odds_outfile << to_json(wp) << std::endl;
    }
  } catch (cli_option_help &e) {
    return 1;
  } catch (cli_option_argument_not_found &e) {
    std::cout << e.what() << std::endl;
    return 1;
  } catch (cli_option_not_recognized &e) {
    std::cout << e.what() << std::endl;
    return 1;
  } catch (cli_option_not_initialized &e) {
    std::cout << e.what() << std::endl;
    std::cout << cli_options_t::help();
    return 1;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
