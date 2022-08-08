#include "debug.h"
#include "mcmc.hpp"
#include "model.hpp"
#include "program_options.hpp"
#include "sampler.hpp"
#include "tournament.hpp"

#include <csv.h>
#include <vector>

static auto make_dummy_data(size_t team_count, uint64_t seed)
    -> std::vector<match_t> {
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

auto create_name_map(std::vector<std::string> team_names) -> team_name_map_t {
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

static auto parse_match_file(const std::string     &match_filename,
                             const team_name_map_t &name_map)
    -> std::vector<match_t> {
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

static auto get_lh_model(const program_options_t    &program_options,
                         const std::vector<match_t> &matches)
    -> std::tuple<std::unique_ptr<likelihood_model_t>,
                  std::function<params_t(const params_t &, random_engine_t &)>,
                  std::function<double(const params_t &)>> {
  if (program_options.mcmc_options.model_type == likelihood_model::poisson) {
    debug_string(EMIT_LEVEL_IMPORTANT, "Using a Poisson likelihood model");
    std::unique_ptr<likelihood_model_t> lhm =
        std::make_unique<poisson_likelihood_model_t>(
            poisson_likelihood_model_t(matches));
    auto update_func = update_poission_model_factory(1.0);
    return std::make_tuple(std::move(lhm), update_win_probs, uniform_prior);
  }
  debug_string(EMIT_LEVEL_IMPORTANT, "Using the simple likelihood model");
  std::unique_ptr<likelihood_model_t> lhm =
      std::make_unique<simple_likelihood_model_t>(
          simple_likelihood_model_t(matches));
  auto update_func = update_win_probs;
  return std::make_tuple(std::move(lhm), update_func, uniform_prior);
}

void compute_tournament(const program_options_t &program_options) {
  auto team_name_map = create_name_map(program_options.teams);

  auto &output_prefix = program_options.output_prefix;

  if (program_options.input_formats.odds_filename.has_value()) {
    matrix_t odds;
    odds = parse_odds_file(program_options.input_formats.odds_filename.value(),
                           team_name_map);
    const std::string output_suffix = ".odds.json";
    if (program_options.run_modes.single) {
      std::ofstream odds_outfile(output_prefix + ".single" + output_suffix);
      auto          t = tournament_factory_single(program_options.teams);
      t.reset_win_probs(odds);
      auto wp = t.eval();
      odds_outfile << to_json(wp) << std::endl;
    }
    if (program_options.run_modes.dynamic) {
      std::ofstream odds_outfile(output_prefix + ".dynamic" + output_suffix);
      auto          t = tournament_factory(program_options.teams);
      t.reset_win_probs(odds);
      auto wp = t.eval();
      odds_outfile << to_json(wp) << std::endl;
    }
    if (program_options.run_modes.simulation) {
      std::ofstream odds_outfile(output_prefix + ".sim" + output_suffix);
      auto          t = tournament_factory_simulation(program_options.teams);
      t.reset_win_probs(odds);
      size_t iters = program_options.simulation_options.samples;
      auto   wp    = t.eval(iters);
      odds_outfile << to_json(wp) << std::endl;
    }
  }

  if (program_options.input_formats.probs_filename.has_value()) {
    matrix_t probs = parse_prob_files(
        program_options.input_formats.probs_filename.value(), team_name_map);
    const std::string output_suffix = ".probs.json";
    if (program_options.run_modes.single) {
      std::ofstream probs_outfile(output_prefix + ".single" + output_suffix);
      auto          t = tournament_factory_single(program_options.teams);
      t.reset_win_probs(probs);
      auto wp = t.eval();
      probs_outfile << to_json(wp) << std::endl;
    }
    if (program_options.run_modes.dynamic) {
      std::ofstream probs_outfile(output_prefix + ".dynamic" + output_suffix);
      auto          t = tournament_factory(program_options.teams);
      t.reset_win_probs(probs);
      auto wp = t.eval();
      probs_outfile << to_json(wp) << std::endl;
    }
    if (program_options.run_modes.simulation) {
      std::ofstream probs_outfile(output_prefix + ".sim" + output_suffix);
      auto          t = tournament_factory_simulation(program_options.teams);
      t.reset_win_probs(probs);
      size_t iters = program_options.simulation_options.samples;
      auto   wp    = t.eval(iters);
      probs_outfile << to_json(wp) << std::endl;
    }
  }
}

void mcmc_run(const program_options_t &program_options) {
  auto team_name_map = create_name_map(program_options.teams);

  std::vector<match_t> matches;
  if (program_options.input_formats.matches_filename.has_value()) {
    if (program_options.input_formats.dummy) {
      debug_string(EMIT_LEVEL_IMPORTANT, "Making dummy data");
      matches =
          make_dummy_data(program_options.teams.size(), program_options.seed);
    } else {
      matches = parse_match_file(
          program_options.input_formats.matches_filename.value(),
          team_name_map);
    }
  }

  const auto       &output_prefix = program_options.output_prefix;
  const std::string output_suffix = ".json";

  size_t mcmc_samples = program_options.mcmc_options.samples;
  size_t burnin_samples =
      static_cast<double>(mcmc_samples) * program_options.mcmc_options.burnin;

  if (program_options.run_modes.single) {
    auto [lhm, update_func, prior_func] =
        get_lh_model(program_options, matches);
    sampler_t<single_node_t> sampler{
        std::move(lhm), tournament_factory_single(program_options.teams)};

    debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler (Single Mode)");
    sampler.run_chain(
        mcmc_samples, program_options.seed, update_func, prior_func);
    auto summary = sampler.summary();

    write_summary(summary,
                  output_prefix,
                  std::string{".single"},
                  output_suffix,
                  burnin_samples);
  }

  if (program_options.run_modes.dynamic) {
    auto [lhm, update_func, prior_func] =
        get_lh_model(program_options, matches);
    sampler_t<tournament_node_t> sampler{
        std::move(lhm), tournament_factory(program_options.teams)};

    debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler (Dynamic Mode)");
    sampler.run_chain(
        mcmc_samples, program_options.seed, update_func, prior_func);
    auto summary = sampler.summary();

    write_summary(summary,
                  output_prefix,
                  std::string{".dynmic"},
                  output_suffix,
                  burnin_samples);
  }

  if (program_options.run_modes.simulation) {
    auto [lhm, update_func, prior_func] =
        get_lh_model(program_options, matches);
    sampler_t<simulation_node_t> sampler{
        std::move(lhm), tournament_factory_simulation(program_options.teams)};

    sampler.set_simulation_iterations(
        program_options.simulation_options.samples);

    debug_string(EMIT_LEVEL_PROGRESS, "Running MCMC sampler (Simulation Mode)");
    sampler.run_chain(
        mcmc_samples, program_options.seed, update_func, prior_func);
    auto summary = sampler.summary();
    write_summary(summary,
                  output_prefix,
                  std::string{".sim"},
                  output_suffix,
                  burnin_samples);
  }
}
