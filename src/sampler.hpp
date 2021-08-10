#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "dataset.hpp"
#include "debug.h"
#include "summary.hpp"
#include "tournament_node.hpp"
#include <random>
#include <utility>
#include <vector>

template <typename result_type> class beta_distribution {
public:
  beta_distribution() = delete;
  beta_distribution(result_type alpha,
                    result_type beta,
                    result_type theta = 1.0) :
      g1{alpha, theta}, g2{beta, theta} {}
  template <class generator_type> result_type operator()(generator_type &gen) {
    result_type x = g1(gen);
    result_type y = g2(gen);
    return x / (x + y);
  }

private:
  std::gamma_distribution<result_type> g1;
  std::gamma_distribution<result_type> g2;
};

/**
 * Generate an a and b suitable for configuring a beta distrbution. The median
 * of the distrbution will be `median`, and the concentration will be `k`.
 */
constexpr inline std::pair<double, double> make_ab(double median, double k) {
  double a = median * (k - 2) + 1;
  double b = k - a;
  return std::make_pair(a, b);
}

/**
 * Class which will perform the MCMC search given a dataset and a tournament.
 */
template <typename T> class sampler_t {
public:
  sampler_t(const dataset_t &ds, tournament_t<T> &&t) :
      _dataset{ds}, _tournament{std::move(t)} {}

  std::vector<result_t> report() const { return _samples; }

  /**
   * Return a summary of the samples. For more information, please see the
   * `summary_t` class.
   */
  summary_t summary() const { return summary_t{_samples}; }

  void run_chain(size_t iters, unsigned int seed) {
    size_t   team_count = _tournament.tip_count();
    params_t params(team_count);
    params_t temp_params{params};
    _samples.clear();
    _samples.reserve(iters);
    std::mt19937_64                  gen(seed);
    std::uniform_real_distribution<> coin(0.0, 1.0);

    {
      beta_distribution<double> beta_dis(1, 1);
      for (auto &f : params) { f = beta_dis(gen); }
    }

    double cur_lh = _dataset.log_likelihood(params);
    for (size_t i = 0; i < iters; ++i) {
      for (size_t j = 0; j < params.size(); ++j) {
        auto [a, b] = make_ab(params[j], 5);
        beta_distribution<double> bd(a, b);
        temp_params[j] = bd(gen);
        debug_print(EMIT_LEVEL_DEBUG,
                    "used a: %f, b: %f to gen %f",
                    a,
                    b,
                    temp_params[j]);
      }

      double next_lh = _dataset.log_likelihood(temp_params);
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

  vector_t run_simulation(const params_t &params) {
    _tournament.reset_win_probs(normalize_params(params));
    return _tournament.eval();
  }

  void record_sample(const params_t &params, double llh) {
    result_t r{run_simulation(params), params, llh};
    _samples.emplace_back(r);
  }

  dataset_t             _dataset;
  tournament_t<T>       _tournament;
  std::vector<result_t> _samples;
};

#endif
