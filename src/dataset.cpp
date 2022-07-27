#include "dataset.hpp"

#include "debug.h"
#include "match.hpp"
#include "util.hpp"
#include <cmath>
#include <numeric>
#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * Construct a dataset class given a list of matches. The matches are of a
 * `match_t` type, and encode a match played between the two teams.
 */
simple_likelihood_model_t::simple_likelihood_model_t(
    const std::vector<match_t> &matches) :
    _team_count(count_teams(matches)) {

  _win_matrix.reserve(_team_count);
  for (size_t i = 0; i < _team_count; ++i) {
    _win_matrix.emplace_back(_team_count);
  }

  for (const auto &m : matches) {
    _win_matrix[m.r_team][m.l_team] += 1 * static_cast<unsigned int>(m.winner);
    _win_matrix[m.l_team][m.r_team] += 1 * static_cast<unsigned int>(!m.winner);
  }
}

/**
 * Using the list of matches, this function computes the likelhood of the
 * proposed win probabilities.
 */
auto simple_likelihood_model_t::log_likelihood(
    const params_t &team_win_probs) const -> double {
  double llh = 0.0;
  debug_print(EMIT_LEVEL_DEBUG,
              "team_win_probs: %s",
              to_string(team_win_probs).c_str());
  debug_print(EMIT_LEVEL_DEBUG, "win matrix size: %lu", _win_matrix.size());

  for (size_t i = 0; i < _win_matrix.size(); ++i) {
    for (size_t j = i + 1; j < _win_matrix.size(); ++j) {
      double l_wp = team_win_probs[i] / (team_win_probs[i] + team_win_probs[j]);
      double r_wp = 1 - l_wp;
      debug_print(EMIT_LEVEL_DEBUG,
                  "twp[i]: %f, twp[j]: %f, l_wp: %f, r_wp: %f i: %lu, j: %lu",
                  team_win_probs[i],
                  team_win_probs[j],
                  l_wp,
                  r_wp,
                  i,
                  j);
      double tmp_lh = int_pow(l_wp, _win_matrix[i][j]) *
                      int_pow(r_wp, _win_matrix[j][i]) *
                      combinations(_win_matrix[i][j] + _win_matrix[j][i],
                                   _win_matrix[i][j]);
      llh += std::log(tmp_lh);
    }
  }
  debug_print(EMIT_LEVEL_DEBUG, "computed llh: %f", llh);
  assert_string(!std::isnan(llh), "LH computed is NaN");
  return llh;
}

auto poisson_likelihood_model_t::log_likelihood(const params_t &team_strs) const
    -> double {
  double llh = 0.0;

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (const auto &m : _matches) {
    double param1 = NAN;
    double param2 = NAN;
    param1        = team_strs[m.l_team];
    param2        = team_strs[m.r_team];

    double lambda_l = std::exp(param1 - param2);
    double lambda_r = std::exp(param2 - param1);

    double term_l = std::pow(lambda_l, m.l_goals) / factorial(m.l_goals) *
                    std::exp(-lambda_l);

    double term_r = std::pow(lambda_r, m.r_goals) / factorial(m.r_goals) *
                    std::exp(-lambda_r);

#ifdef _OPENMP
#pragma omp critical
#endif
    llh += std::log(term_r * term_l);
  }

  assert_string(!std::isnan(llh), "LH computed is NaN");
  return llh;
}

auto simple_likelihood_model_t::generate_win_probs(const params_t &params) const
    -> matrix_t {
  matrix_t wp;
  wp.reserve(_team_count);
  for (size_t i = 0; i < _team_count; ++i) { wp.emplace_back(_team_count); }
  for (size_t i = 0; i < _team_count; ++i) {
    for (size_t j = i + 1; j < _team_count; ++j) {
      double w = params[i] / (params[i] + params[j]);
      assert_string(w <= 1.0, "Win prob is well formed");
      assert_string(w >= 0.0, "Win prob is well formed");
      wp[i][j] = w;
      wp[j][i] = 1 - w;
    }
  }
  return wp;
}

auto poisson_likelihood_model_t::generate_win_probs(
    const params_t &params) const -> matrix_t {
  matrix_t wp;
  wp.reserve(_team_count);
  for (size_t i = 0; i < _team_count; ++i) { wp.emplace_back(_team_count); }

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (size_t i = 0; i < _team_count; i++) {
    for (size_t j = i + 1; j < _team_count; j++) {
      double param1 = params[i];
      double param2 = params[j];

      double lamda1 = std::exp(param1 - param2);
      double lamda2 = std::exp(param2 - param1);

      double t1_prob  = skellam_cmf(-1, lamda2, lamda1);
      double tie_prob = skellam_pmf(0, lamda2, lamda1);
      double t2_prob  = 1 - t1_prob - tie_prob;

      t1_prob += tie_prob / 2.0;
      t2_prob += tie_prob / 2.0;

      assert_string(t1_prob <= 1.0,
                    "Generated probabilities are not well formed");
      assert_string(t1_prob >= 0.0,
                    "Generated probabilities are not well formed");

      assert_string(t2_prob <= 1.0,
                    "Generated probabilities are not well formed");
      assert_string(t2_prob >= 0.0,
                    "Generated probabilities are not well formed");

      wp[i][j] = t1_prob;
      wp[j][i] = t2_prob;
    }
  }

  return wp;
}
