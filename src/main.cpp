#include <algorithm>
#include <chrono>
#include <cstdint>
#include <csv.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "cli.hpp"
#include "dataset.hpp"
#include "debug.h"
#include "sampler.hpp"
#include "single_node.hpp"
#include "summary.hpp"
#include "tournament.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"
#include "util.hpp"
int DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_PROGRESS;

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
constexpr char GIT_REV_STRING[]    = STRINGIFY(GIT_REV);
constexpr char GIT_COMMIT_STRING[] = STRINGIFY(GIT_COMMIT);
constexpr char BUILD_DATE_STRING[] = STRINGIFY(BUILD_DATE);

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
using duration_t = std::chrono::duration<double>;

using team_name_map_t = std::unordered_map<std::string, size_t>;

static void print_version() {
  debug_string(EMIT_LEVEL_IMPORTANT, "Running Phylourny");
  debug_print(EMIT_LEVEL_IMPORTANT, "Version: %s", GIT_REV_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Commit: %s", GIT_COMMIT_STRING);
  debug_print(EMIT_LEVEL_IMPORTANT, "Build Date: %s", BUILD_DATE_STRING);
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
  debug_print(
      EMIT_LEVEL_IMPORTANT, "Run Finished, time: %fs", duration.count());
}

static auto make_dummy_data(size_t team_count, uint64_t seed) -> std::vector<match_t> {
  std::vector<match_t>                  matches{};
  std::mt19937_64                       gen(seed);
  std::uniform_int_distribution<size_t> team(0, team_count - 1);
  std::exponential_distribution         team_str_dist(0.75);

  std::vector<double> params(team_count);
  std::generate(
      params.begin(), params.end(), [&]() { return team_str_dist(gen); });
  // for (auto &p : params) { p = team_str_dist(gen); }

  debug_print(
      EMIT_LEVEL_IMPORTANT, "Team strengths are %s", to_json(params).c_str());

  for (size_t i = 0; i < 40; ++i) {
    size_t t1 = team(gen);
    size_t t2 = t1;
    while (t2 == t1) { t2 = team(gen); }

    std::poisson_distribution<size_t> t1d(params[t1]);
    std::poisson_distribution<size_t> t2d(params[t2]);

    size_t goals1 = 0;
    size_t goals2 = 0;
    while (goals1 == goals2) {
      goals1 = t1d(gen);
      goals2 = t2d(gen);
    }

    matches.push_back(
        {t1,
         t2,
         goals1,
         goals2,
         goals1 < goals2 ? match_winner_t::left : match_winner_t::right});
  }

  return matches;
}

static auto create_name_map(std::vector<std::string> team_names) -> team_name_map_t {
  team_name_map_t name_map;

  for (size_t i = 0; i < team_names.size(); i++) {
    name_map[team_names[i]] = i;
  }

  return name_map;
}

static auto parse_odds_file(const std::string     &odds_filename,
                                const team_name_map_t &name_map) -> matrix_t {

  matrix_t odds;

  odds.resize(name_map.size());
  for (auto &&r : odds) { r.resize(name_map.size()); }

  io::CSVReader<4> odds_file(odds_filename);
  odds_file.read_header(
      io::ignore_extra_column, "team1", "team2", "odds1", "odds2");

  std::string team1;
  std::string team2;
  double      odds1 = NAN;
  double      odds2 = NAN;
  while (odds_file.read_row(team1, team2, odds1, odds2)) {
    size_t index1 = name_map.at(team1);
    size_t index2 = name_map.at(team2);

    double prob = odds1 / (odds1 + odds2);

    odds[index1][index2] = prob;
    odds[index2][index1] = 1 - prob;
  }

  return odds;
}

static auto parse_match_file(const std::string &match_filename,
                                             const team_name_map_t &name_map) -> std::vector<match_t> {
  std::vector<match_t> match_history;

  io::CSVReader<4> match_file(match_filename);
  match_file.read_header(
      io::ignore_extra_column, "team1", "team2", "team1-goals", "team2-goals");
  std::string team1;
  std::string team2;
  size_t      team1_goals = 0;
  size_t      team2_goals = 0;
  while (match_file.read_row(team1, team2, team1_goals, team2_goals)) {
    size_t index1 = name_map.at(team1);
    size_t index2 = name_map.at(team2);
    match_history.push_back({index1,
                             index2,
                             team1_goals,
                             team2_goals,
                             team1_goals < team2_goals
                                 ? match_winner_t::left
                                 : match_winner_t::right});
  }

  return match_history;
}

static auto parse_prob_files(const std::string     &probs_filename,
                                 const team_name_map_t &name_map) -> matrix_t {

  matrix_t win_probs;
  win_probs.resize(name_map.size());

  for (auto &&v : win_probs) { v.resize(name_map.size()); }

  io::CSVReader<3> probs_file(probs_filename);
  probs_file.read_header(
      io::ignore_extra_column, "team1", "team2", "prob-win-team1");
  std::string team1;
  std::string team2;
  double      win_prob = 0.0;
  while (probs_file.read_row(team1, team2, win_prob)) {
    size_t team1_index                  = name_map.at(team1);
    size_t team2_index                  = name_map.at(team2);
    win_probs[team1_index][team2_index] = win_prob;
    win_probs[team2_index][team1_index] = 1.0 - win_prob;
  }

  return win_probs;
}

static void write_summary(const summary_t   &summary,
                          const std::string &output_prefix,
                          const std::string &output_infix,
                          const std::string &output_suffix,
                          size_t             burnin_samples) {
  std::ofstream outfile(output_prefix + output_infix + ".samples" +
                        output_suffix);
  summary.write_samples(outfile, 0, 1);

  std::ofstream mlp_outfile(output_prefix + output_infix + ".mlp" +
                            output_suffix);
  summary.write_mlp(mlp_outfile, burnin_samples);

  std::ofstream mmpp_outfile(output_prefix + output_infix + ".mmpp" +
                             output_suffix);
  summary.write_mmpp(mmpp_outfile, burnin_samples);
}

static auto
get_lh_model(const cli_options_t        &cli_options,
             const std::vector<match_t> &matches) -> std::tuple<std::unique_ptr<likelihood_model_t>,
                  std::function<params_t(const params_t &, random_engine_t &)>,
                  std::function<double(const params_t &)>> {
  if (cli_options["poisson"].value(true)) {
    debug_string(EMIT_LEVEL_IMPORTANT, "Using a Poisson likelihood model");
    std::unique_ptr<likelihood_model_t> lhm =
        std::make_unique<poisson_likelihood_model_t>(
            poisson_likelihood_model_t(matches));
    auto update_func = update_poission_model_factory(1.0);
    return std::make_tuple(std::move(lhm), update_func, normal_prior);
  }     debug_string(EMIT_LEVEL_IMPORTANT, "Using the simple likelihood model");
    std::unique_ptr<likelihood_model_t> lhm =
        std::make_unique<simple_likelihood_model_t>(
            simple_likelihood_model_t(matches));
    auto update_func = update_win_probs;
    return std::make_tuple(std::move(lhm), update_func, uniform_prior);
 
}

static void mcmc_run(const cli_options_t            &cli_options,
                     const std::vector<std::string> &teams) {
  auto team_name_map = create_name_map(teams);

  std::vector<match_t> matches;
  if (cli_options["matches"].initialized()) {
    matches = parse_match_file(cli_options["matches"].value<std::string>(),
                               team_name_map);
  } else if (cli_options["dummy"].value<bool>(false)) {
    debug_string(EMIT_LEVEL_IMPORTANT, "Making dummy data");
    matches =
        make_dummy_data(teams.size(), cli_options["seed"].value<uint64_t>());
  }

  const auto output_prefix = cli_options["prefix"].value<std::string>();
  const std::string output_suffix = ".json";

  size_t mcmc_samples   = cli_options["samples"].value(10'000'000LU);
  size_t burnin_samples = static_cast<size_t>(
      static_cast<double>(mcmc_samples) * cli_options["burnin"].value(0.1));

  if (cli_options["single"].value(false)) {
    auto [lhm, update_func, prior_func] = get_lh_model(cli_options, matches);
    sampler_t<single_node_t> sampler{std::move(lhm),
                                     tournament_factory_single(teams)};

    debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler (Single Mode)");
    sampler.run_chain(mcmc_samples,
                      cli_options["seed"].value<uint64_t>(),
                      update_win_probs,
                      prior_func);
    auto summary = sampler.summary();

    write_summary(summary,
                  output_prefix,
                  std::string{".single"},
                  output_suffix,
                  burnin_samples);
  }

  if (cli_options["dynamic"].value(true)) {
    auto [lhm, update_func, prior_func] = get_lh_model(cli_options, matches);
    sampler_t<tournament_node_t> sampler{std::move(lhm),
                                         tournament_factory(teams)};

    debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler (Dynamic Mode)");
    sampler.run_chain(mcmc_samples,
                      cli_options["seed"].value<uint64_t>(),
                      update_win_probs,
                      prior_func);
    auto summary = sampler.summary();

    write_summary(summary,
                  output_prefix,
                  std::string{".dynmic"},
                  output_suffix,
                  burnin_samples);
  }

  if (cli_options["sim"].value(false)) {
    auto [lhm, update_func, prior_func] = get_lh_model(cli_options, matches);
    sampler_t<simulation_node_t> sampler{std::move(lhm),
                                         tournament_factory_simulation(teams)};

    sampler.set_simulation_iterations(
        cli_options["sim-iters"].value(1'000'000LU));

    debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler (Simulation Mode)");
    sampler.run_chain(mcmc_samples,
                      cli_options["seed"].value<uint64_t>(),
                      update_win_probs,
                      prior_func);
    auto summary = sampler.summary();
    write_summary(summary,
                  output_prefix,
                  std::string{".sim"},
                  output_suffix,
                  burnin_samples);
  }
}

static void compute_tournament(const cli_options_t            &cli_options,
                               const std::vector<std::string> &teams) {
  auto team_name_map = create_name_map(teams);

  const auto output_prefix = cli_options["prefix"].value<std::string>();

  if (cli_options["odds"].initialized()) {
    matrix_t odds;
    odds = parse_odds_file(cli_options["odds"].value<std::string>(),
                           team_name_map);
    const std::string output_suffix = ".odds.json";
    if (cli_options["single"].value(false)) {
      std::ofstream odds_outfile(output_prefix + ".single" + output_suffix);
      auto          t = tournament_factory_single(teams);
      t.reset_win_probs(odds);
      auto wp = t.eval();
      odds_outfile << to_json(wp) << std::endl;
    }
    if (cli_options["dynamic"].value(true)) {
      std::ofstream odds_outfile(output_prefix + ".dynamic" + output_suffix);
      auto          t = tournament_factory(teams);
      t.reset_win_probs(odds);
      auto wp = t.eval();
      odds_outfile << to_json(wp) << std::endl;
    }
    if (cli_options["sim"].value(false)) {
      std::ofstream odds_outfile(output_prefix + ".sim" + output_suffix);
      auto          t = tournament_factory_simulation(teams);
      t.reset_win_probs(odds);
      size_t iters = cli_options["sim-iters"].value(1'000'000LU);
      auto   wp    = t.eval(iters);
      odds_outfile << to_json(wp) << std::endl;
    }
  }

  if (cli_options["probs"].initialized()) {
    matrix_t probs = parse_prob_files(cli_options["probs"].value<std::string>(),
                                      team_name_map);
    const std::string output_suffix = ".probs.json";
    if (cli_options["single"].value(false)) {
      std::ofstream probs_outfile(output_prefix + ".single" + output_suffix);
      auto          t = tournament_factory_single(teams);
      t.reset_win_probs(probs);
      auto wp = t.eval();
      probs_outfile << to_json(wp) << std::endl;
    }
    if (cli_options["dynamic"].value(true)) {
      std::ofstream probs_outfile(output_prefix + ".dynamic" + output_suffix);
      auto          t = tournament_factory(teams);
      t.reset_win_probs(probs);
      auto wp = t.eval();
      probs_outfile << to_json(wp) << std::endl;
    }
    if (cli_options["sim"].value(false)) {
      std::ofstream probs_outfile(output_prefix + ".sim" + output_suffix);
      auto          t = tournament_factory_simulation(teams);
      t.reset_win_probs(probs);
      size_t iters = cli_options["sim-iters"].value(1'000'000LU);
      auto   wp    = t.eval(iters);
      probs_outfile << to_json(wp) << std::endl;
    }
  }
}

auto main(int argc, char **argv) -> int {
  DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_PROGRESS;
  auto start_time       = std::chrono::high_resolution_clock::now();
  print_version();
  try {
    cli_options_t cli_options{argc, argv};

    if (cli_options["debug"].value<bool>(false)) {
      DEBUG_VERBOSITY_LEVEL = EMIT_LEVEL_DEBUG;
    }

    std::vector<std::string> teams;

    {
      std::ifstream teams_file(cli_options["teams"].value<std::string>());
      for (std::string tmp; std::getline(teams_file, tmp);) {
        if (tmp.empty()) { continue; }
        teams.push_back(tmp);
      }
    }

    auto team_name_map = create_name_map(teams);

    /*
     * This janky if statement is so that we don't read from the random device
     * if we don't need to. I know, it's not really that important, but it
     * matters to me lol.
     */
    if (!cli_options["seed"].initialized()) {
      std::random_device rd;
      cli_options["seed"].set_if_unset<uint64_t>(rd());
      assert_string(cli_options["seed"].initialized(),
                    "CLI Option seed shoudl be initilaized ehre");
    }

    print_run_info(cli_options);

    if (cli_options["matches"].initialized() ||
        cli_options["dummy"].initialized()) {
      mcmc_run(cli_options, teams);
    }

    else if (cli_options["probs"].initialized() ||
             cli_options["odds"].initialized()) {
      compute_tournament(cli_options, teams);
    }

  } catch (cli_option_help &e) {
    (void)e;
    return 1;
  } catch (cli_option_exception &e) {
    std::cout << e.what() << std::endl;
    std::cout << cli_options_t::help();
    return 1;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  print_end_time(start_time, end_time);
  return 0;
}
