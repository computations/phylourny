#include "debug.h"
#include "factorial.hpp"
#include "match.hpp"
#include "model.hpp"
#include "program_options.hpp"
#include "util.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
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
    _param_count(count_teams(matches)) {

  _win_matrix.reserve(_param_count);
  for (size_t i = 0; i < _param_count; ++i) {
    _win_matrix.emplace_back(_param_count);
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
#pragma omp parallel for reduction(+ : llh)
#endif
  for (const auto &m : _matches) {
    /*
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
    */

    double param1      = team_strs[m.l_team];
    double param2      = team_strs[m.r_team];
    double scale_param = team_strs[team_strs.size() - 1];

    double log_lambda_l = param1 - param2 + scale_param;
    double log_lambda_r = param2 - param1 + scale_param;

    double term_l = log_lambda_l * m.l_goals - log_factorial(m.l_goals) -
                    std::exp(log_lambda_l);
    double term_r = log_lambda_r * m.r_goals - log_factorial(m.r_goals) -
                    std::exp(log_lambda_r);

    double term = term_l + term_r;
    assert_string(!std::isnan(term), "Term computed is nan");
    llh += term;
  }

  assert_string(!std::isnan(llh), "LLH computed is NaN");
  assert_string(llh <= 0.0, "LLH is positive");
  return llh;
}

auto simple_likelihood_model_t::generate_win_probs(
    const params_t &params, const std::vector<size_t> &team_indicies) const
    -> matrix_t {
  matrix_t wp;
  wp.reserve(team_indicies.size());
  for (size_t i = 0; i < team_indicies.size(); ++i) {
    wp.emplace_back(team_indicies.size());
  }
  for (size_t i = 0; i < team_indicies.size(); ++i) {
    for (size_t j = i + 1; j < team_indicies.size(); ++j) {
      size_t team1_index = team_indicies[i];
      size_t team2_index = team_indicies[j];
      double w =
          params[team1_index] / (params[team1_index] + params[team2_index]);
      assert_string(w <= 1.0, "Win prob is well formed");
      assert_string(w >= 0.0, "Win prob is well formed");
      wp[i][j] = w;
      wp[j][i] = 1 - w;
    }
  }
  return wp;
}

auto poisson_likelihood_model_t::generate_win_probs(
    const params_t &params, const std::vector<size_t> &team_indicies) const
    -> matrix_t {
  matrix_t wp;
  wp.reserve(team_indicies.size());
  for (size_t i = 0; i < team_indicies.size(); ++i) {
    wp.emplace_back(team_indicies.size());
  }

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (size_t i = 0; i < team_indicies.size(); i++) {
    for (size_t j = i + 1; j < team_indicies.size(); j++) {
      double team1_index = team_indicies[i];
      double team2_index = team_indicies[j];
      double param1      = params[team1_index];
      double param2      = params[team2_index];

      double lambda1 = std::exp(param1 - param2);
      double lambda2 = std::exp(param2 - param1);

      double t1_prob  = skellam_cmf(-1, lambda2, lambda1);
      double tie_prob = skellam_pmf(0, lambda2, lambda1);
      double t2_prob  = 1 - t1_prob - tie_prob;

      t1_prob += tie_prob / 2.0;
      t2_prob += tie_prob / 2.0;

      t1_prob = phylourny_prob_clamp(t1_prob);
      t2_prob = phylourny_prob_clamp(t2_prob);

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
