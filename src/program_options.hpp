#ifndef PROGRAM_OPTIONS_HPP
#define PROGRAM_OPTIONS_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

enum class likelihood_model { simple, poisson };

struct input_format_options_t {
  std::optional<std::string> odds_filename;
  std::optional<std::string> probs_filename;
  std::optional<std::string> matches_filename;
  std::optional<std::string> bestofs_filename;
  bool                       dummy;
};

enum class run_mode_e {
  single,
  dynamic,
  simulation,
};

constexpr inline std::string_view describe_run_type(run_mode_e rt) {
  switch (rt) {
  case run_mode_e::single:
    return "single";
  case run_mode_e::dynamic:
    return "dynamic";
  case run_mode_e::simulation:
    return "simulation";
  default:
    return "unknown";
  }
}

struct simulation_mode_options_t {
  size_t samples;
};

struct mcmc_options_t {
  size_t           samples;
  double           burnin;
  bool             sample_matrix;
  likelihood_model model_type;
  bool             node_probabilites;
};

struct program_options_t {
  std::string              output_prefix;
  std::vector<std::string> teams;

  size_t seed;

  input_format_options_t input_formats;
  run_mode_e             run_mode;

  simulation_mode_options_t simulation_options;
  mcmc_options_t            mcmc_options;
};

void run(const program_options_t &);

#endif
