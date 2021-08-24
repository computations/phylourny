#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "dataset.hpp"
#include "debug.h"
#include "summary.hpp"
#include "tournament_node.hpp"
#include <functional>
#include <memory>
#include <random>
#include <utility>
#include <vector>

/**
 * Class which will perform the MCMC search given a dataset and a tournament.
 */
template <typename T> class sampler_t {
public:
  sampler_t(std::unique_ptr<likelihood_model_t> &&lhm, tournament_t<T> &&t) :
      _lh_model{std::move(lhm)}, _tournament{std::move(t)} {}

  std::vector<result_t> report() const { return _samples; }

  /**
   * Return a summary of the samples. For more information, please see the
   * `summary_t` class.
   */
  summary_t summary() const { return summary_t{_samples}; }

  void run_chain(
      size_t       iters,
      unsigned int seed,
      const std::function<params_t(const params_t &, random_engine_t &gen)>
          &update_func) {
    params_t params(_lh_model->param_count());
    params_t temp_params{params};
    _samples.clear();
    _samples.reserve(iters);
    random_engine_t                  gen(seed);
    std::uniform_real_distribution<> coin(0.0, 1.0);

    {
      beta_distribution<double> beta_dis(1, 1);
      for (auto &f : params) { f = beta_dis(gen); }
    }

    double cur_lh = _lh_model->log_likelihood(params);
    for (size_t i = 0; i < iters; ++i) {
      if (i % 10000 == 0 && i != 0) {
        debug_print(EMIT_LEVEL_PROGRESS, "%lu samples", i);
      }

      temp_params = update_func(params, gen);

      double next_lh = _lh_model->log_likelihood(temp_params);
      debug_print(
          EMIT_LEVEL_DEBUG, "tmp_params: %s", to_string(temp_params).c_str());
      debug_print(EMIT_LEVEL_DEBUG,
                  "next_lh : %f, cur_lh:%f, acceptance ratio: %f",
                  next_lh,
                  cur_lh,
                  next_lh / cur_lh);
      if (std::isnan(next_lh)) { throw std::runtime_error("next_lh is nan"); }
      if (coin(gen) < std::exp(next_lh - cur_lh)) {
        record_sample(temp_params, next_lh);
        std::swap(next_lh, cur_lh);
        std::swap(temp_params, params);
      }
    }
  }

  void set_simulation_iterations(size_t s) { _simulation_iterations = s; }

private:
  matrix_t normalize_params(const params_t &params) {
    matrix_t wp;
    size_t   tip_count = _tournament.tip_count();
    wp.reserve(tip_count);
    for (size_t i = 0; i < tip_count; ++i) { wp.emplace_back(tip_count); }
    for (size_t i = 0; i < tip_count; ++i) {
      for (size_t j = i + 1; j < tip_count; ++j) {
        double w = params[i] / (params[i] + params[j]);
        wp[i][j] = w;
        wp[j][i] = 1 - w;
      }
    }
    return wp;
  }

  vector_t run_simulation(const params_t &);

  void record_sample(const params_t &params, double llh) {
    result_t r{run_simulation(params), params, llh};
    _samples.emplace_back(r);
  }

  std::unique_ptr<likelihood_model_t> _lh_model;
  tournament_t<T>                     _tournament;
  std::vector<result_t>               _samples;
  size_t                              _simulation_iterations;
};

template <typename T1>
vector_t sampler_t<T1>::run_simulation(const params_t &params) {
  _tournament.reset_win_probs(normalize_params(params));
  return _tournament.eval();
}

template <>
vector_t sampler_t<simulation_node_t>::run_simulation(const params_t &params) {
  _tournament.reset_win_probs(normalize_params(params));
  return _tournament.eval(_simulation_iterations);
}

#endif
