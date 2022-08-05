#include "debug.h"
#include "mcmc.hpp"

void mcmc_run(const cli_options_t            &cli_options,
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

  const auto        output_prefix = cli_options["prefix"].value<std::string>();
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
                      update_func,
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
                      update_func,
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
                      update_func,
                      prior_func);
    auto summary = sampler.summary();
    write_summary(summary,
                  output_prefix,
                  std::string{".sim"},
                  output_suffix,
                  burnin_samples);
  }
}
