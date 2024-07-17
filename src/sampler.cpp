#include "sampler.hpp"

template <>
vector_t
sampler_t<simulation_node_t>::run_simulation(const matrix_t &prob_matrix) {
  _tournament.reset_win_probs(prob_matrix);
  return _tournament.eval(_simulation_iterations);
}
