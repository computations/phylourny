#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "debug.h"
#include "model.hpp"
#include "summary.hpp"
#include "tournament_node.hpp"
#include "util.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * Class which will perform the MCMC search given a dataset and a tournament.
 */
template <typename T> class sampler_t {
public:
  sampler_t(std::unique_ptr<likelihood_model_t> &&lhm, tournament_t<T> &&t) :
      _lh_model{std::move(lhm)}, _tournament{std::move(t)}, _team_indicies{} {}

  void generate_default_team_indicies() {
    _team_indicies.resize(_tournament.tip_count());
    for (size_t i = 0; i < _team_indicies.size(); ++i) {
      _team_indicies[i] = i;
    }
  }

  void set_team_indicies(const std::vector<size_t> &ti) {
    assert_string(
        ti.size() <= _tournament.tip_count(),
        "Attempted to team indicies with more entries than tournament tips");
    _team_indicies = ti;
  }

  [[nodiscard]] auto report() const -> std::vector<result_t> {
    return _samples;
  }

  /**
   * Return a summary of the samples. For more information, please see the
   * `summary_t` class.
   */
  [[nodiscard]] auto summary() const -> summary_t {
    return summary_t{_samples};
  }

  void run_chain(size_t                                         iters,
                 uint64_t                                       seed,
                 const std::function<std::pair<params_t, double>(
                     const params_t &, random_engine_t &gen)>  &update_func,
                 const std::function<double(const params_t &)> &prior,
                 bool sample_matrix = false) {

    constexpr size_t waiting_time = 100;

    if (iters == 0) {
      throw std::runtime_error{"Iters should be greater than 0"};
    }

    if (_team_indicies.empty()) { generate_default_team_indicies(); }

    params_t params(_lh_model->param_count(), 0.5);
    params_t temp_params{params};
    _samples.clear();
    _samples.reserve(iters);
    random_engine_t                  gen(seed);
    std::uniform_real_distribution<> coin(0.0, 1.0);

    // params = update_func(params, gen);

    size_t successes = 0;

    double cur_lh = _lh_model->log_likelihood(params);
    for (size_t i = 0; _samples.size() < iters; ++i) {

      double hastings_ratio;
      std::tie(temp_params, hastings_ratio) = update_func(params, gen);

      double next_lh = _lh_model->log_likelihood(temp_params);
      debug_print(
          EMIT_LEVEL_DEBUG, "tmp_params: %s", to_string(temp_params).c_str());
      if (std::isnan(next_lh)) { throw std::runtime_error("next_lh is nan"); }

      double prior_ratio = prior(temp_params) / prior(params);

      debug_print(EMIT_LEVEL_DEBUG,
                  "next_lh : %f, cur_lh:%f, prior ratio: %f, hastings ratio: "
                  "%f, acceptance ratio: %f",
                  next_lh,
                  cur_lh,
                  prior_ratio,
                  hastings_ratio,
                  std::exp(next_lh - cur_lh) * prior_ratio);

      if (coin(gen) <
          std::exp(next_lh - cur_lh) * prior_ratio * hastings_ratio) {
        std::swap(next_lh, cur_lh);
        std::swap(temp_params, params);
        successes += 1;
      }
      if (i % waiting_time == 0 && i != 0) {
        record_sample(params, cur_lh, successes, i, iters, sample_matrix);
      }
    }
  }

  void set_simulation_iterations(size_t s) { _simulation_iterations = s; }

private:
  vector_t run_simulation(const matrix_t & /*params*/);
  matrix_t compute_win_probs(const params_t &params) {
    return _lh_model->generate_win_probs(params, _team_indicies);
  }

  void record_sample(const params_t &params,
                     double          llh,
                     size_t          successes,
                     size_t          trials,
                     size_t          iters,
                     bool            sample_matrix = false) {
    auto prob_matrix = compute_win_probs(params);

    result_t r{run_simulation(prob_matrix),
               params,
               sample_matrix ? prob_matrix : std::optional<matrix_t>(),
               llh};

    _samples.emplace_back(r);
    if (_samples.size() % 1000 == 0) {
      debug_print(EMIT_LEVEL_PROGRESS,
                  "%lu samples, ratio: %f, ETC: %.2fh",
                  _samples.size(),
                  static_cast<double>(successes) / trials,
                  progress_macro(_samples.size(), iters));
    }
  }

  std::unique_ptr<likelihood_model_t> _lh_model;
  tournament_t<T>                     _tournament;
  std::vector<result_t>               _samples;
  std::vector<size_t>                 _team_indicies;
  size_t                              _simulation_iterations{0};
};

template <typename T1>
auto sampler_t<T1>::run_simulation(const matrix_t &prob_matrix) -> vector_t {
  _tournament.reset_win_probs(prob_matrix);
  return _tournament.eval();
}

template <>
vector_t
sampler_t<simulation_node_t>::run_simulation(const matrix_t &prob_matrix) {
  _tournament.reset_win_probs(prob_matrix);
  return _tournament.eval(_simulation_iterations);
}

#endif
