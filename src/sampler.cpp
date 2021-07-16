#include "sampler.hpp"

/**
 * Run the chain for `iters`.
 *
 * @param iters The number of iterations to run the chain for
 *
 * @param seed Seed for the random number generator. The 2 things controlled
 * by randomness are the initial guess, and the coin flips when accepting a
 * move.
 */
void sampler_t::run_chain(size_t iters, unsigned int seed) {
  size_t team_count = _tournament.tip_count();
  params_t params(team_count);
  params_t temp_params{params};
  _samples.clear();
  _samples.reserve(iters);
  std::mt19937_64 gen(seed);
  std::uniform_real_distribution<> coin(0.0, 1.0);

  {
    beta_distribution<double> beta_dis(1, 1);
    for (auto &f : params) {
      f = beta_dis(gen);
    }
  }

  double cur_lh = _dataset.log_likelihood(params);
  for (size_t i = 0; i < iters; ++i) {
    for (size_t j = 0; j < params.size(); ++j) {
      auto [a, b] = make_ab(params[j], 5);
      beta_distribution<double> bd(a, b);
      temp_params[j] = bd(gen);
      debug_print(EMIT_LEVEL_DEBUG, "used a: %f, b: %f to gen %f", a, b,
                  temp_params[j]);
    }

    double next_lh = _dataset.log_likelihood(temp_params);
    debug_print(EMIT_LEVEL_DEBUG, "tmp_params: %s",
                to_string(temp_params).c_str());
    debug_print(EMIT_LEVEL_DEBUG,
                "next_lh : %f, cur_lh:%f, acceptance ratio: %f", next_lh,
                cur_lh, next_lh / cur_lh);
    if (std::isnan(next_lh)) {
      throw std::runtime_error("next_lh is nan");
    }
    if (coin(gen) < std::exp(next_lh - cur_lh)) {
      record_sample(temp_params, next_lh);
      std::swap(next_lh, cur_lh);
      std::swap(temp_params, params);
    }
  }
}
