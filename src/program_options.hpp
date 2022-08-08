#ifndef PROGRAM_OPTIONS_HPP
#define PROGRAM_OPTIONS_HPP

#include <optional>
#include <string>
#include <vector>

enum class likelihood_model { simple, poisson };

struct input_format_options_t {
  std::optional<std::string> odds_filename;
  std::optional<std::string> probs_filename;
  std::optional<std::string> matches_filename;
  bool                       dummy;
};

struct run_mode_t {
  bool single;
  bool dynamic;
  bool simulation;
};

struct simulation_mode_options_t {
  size_t samples;
};

struct mcmc_options_t {
  size_t           samples;
  double           burnin;
  likelihood_model model_type;
};

struct program_options_t {
  std::string              output_prefix;
  std::vector<std::string> teams;

  size_t seed;

  input_format_options_t input_formats;
  run_mode_t             run_modes;

  simulation_mode_options_t simulation_options;
  mcmc_options_t            mcmc_options;
};

void run(const program_options_t &);

#endif
