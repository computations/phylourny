#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "debug.h"
#include "model.hpp"
#include "results.hpp"
#include "tournament.hpp"
#include "util.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

  void run_chain(results_t                                     &results,
                 size_t                                         iters,
                 size_t                                         burnin_iters,
                 uint64_t                                       seed,
                 const std::function<std::pair<params_t, double>(
                     const params_t &, random_engine_t &gen)>  &update_func,
                 const std::function<double(const params_t &)> &prior,
                 bool sample_matrix = false,
                 bool node_probs    = false) {

    constexpr size_t waiting_time = 100;

    if (iters == 0) {
      throw std::runtime_error{"Iters should be greater than 0"};
    }

    if (_team_indicies.empty()) { generate_default_team_indicies(); }

    params_t                         params(_lh_model->param_count(), 0.5);
    params_t                         temp_params{params};
    random_engine_t                  gen(seed);
    std::uniform_real_distribution<> coin(0.0, 1.0);

    // params = update_func(params, gen);

    size_t successes = 0;

    double cur_lh = _lh_model->log_likelihood(params);
    for (size_t i = 0; results.sample_count() < iters; ++i) {

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
        record_sample(results,
                      params,
                      cur_lh,
                      successes,
                      i,
                      iters,
                      burnin_iters,
                      sample_matrix,
                      node_probs);
      }
    }
  }

  void set_simulation_iterations(size_t s) { _simulation_iterations = s; }

  void set_bestofs(const std::vector<size_t> &bestofs) {
    auto tips = _tournament.tip_count();
    if (tips != std::pow(2, bestofs.size())) {
      debug_string(EMIT_LEVEL_ERROR,
                   "Mismatch in the bestof size vs tournament size");
    }
    _tournament.set_bestof(bestofs);
  }

  auto get_tournament() const -> tournament_t<T> const&{
    return _tournament;
  }

private:
  vector_t run_simulation(const matrix_t & /*params*/);
  matrix_t compute_win_probs(const params_t &params) {
    return _lh_model->generate_win_probs(params, _team_indicies);
  }

  void record_sample(results_t      &results,
                     const params_t &params,
                     double          llh,
                     size_t          successes,
                     size_t          trials,
                     size_t          iters,
                     size_t          burnin_iters,
                     bool            sample_matrix = false,
                     bool            node_probs    = false) {

    if (iters < burnin_iters) { return; }
    if (iters == burnin_iters && iters != 0) {
      debug_string(EMIT_LEVEL_PROGRESS, "Burnin Complete");
      return;
    }
    auto prob_matrix = compute_win_probs(params);
    auto sim_results = run_simulation(prob_matrix);

    result_t r{sim_results,
               params,
               sample_matrix ? prob_matrix : std::optional<matrix_t>(),
               node_probs
                   ? _tournament.get_node_results()
                   : std::optional<std::unordered_map<std::string, vector_t>>(),
               llh};

    results.add_result(std::move(r));
    if (results.sample_count() % 1000 == 0) {
#ifndef JOKE_BUILD
      debug_print(EMIT_LEVEL_PROGRESS,
                  "%lu samples, ratio: %f, ETC: %.2f hours",
                  results.sample_count(),
                  static_cast<double>(successes) / trials,
                  progress_macro(results.sample_count(), iters));
#else
      debug_print(EMIT_LEVEL_PROGRESS,
                  "%lu samples, ratio: %f, ETC: %.10f millifortnights",
                  results.sample_count(),
                  static_cast<double>(successes) / trials,
                  progress_macro(results.sample_count(), iters));
#endif
    }
  }

  std::unique_ptr<likelihood_model_t> _lh_model;
  tournament_t<T>                     _tournament;
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
sampler_t<simulation_node_t>::run_simulation(const matrix_t &prob_matrix);

#endif
