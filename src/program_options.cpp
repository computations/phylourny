#include "mcmc.hpp"
#include "program_options.hpp"

void run(const program_options_t &program_options) {
  if (program_options.input_formats.matches_filename.has_value() ||
      program_options.input_formats.dummy) {
    mcmc_run(program_options);
  }

  if (program_options.input_formats.probs_filename.has_value() ||
      program_options.input_formats.odds_filename.has_value()) {
    compute_tournament(program_options);
  }
}
