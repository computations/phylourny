#ifndef DATASET_HPP
#define DATASET_HPP

#include "debug.h"
#include "match.hpp"
#include "tournament.hpp"
#include "util.hpp"
#include <random>
#include <vector>

class likelihood_model_t {
public:
  virtual double likelihood(const params_t &) const = 0;

  virtual double log_likelihood(const params_t &p) const {
    return std::log(likelihood(p));
  }

  virtual size_t param_count() const = 0;

  virtual ~likelihood_model_t() = default;

  virtual matrix_t generate_win_probs(const params_t &params) const = 0;
};

/**
 * A `simple_likelihood_model_t` is a class used to compute the likelihood of
 * plausible pairwise win matrices. The major concepts are the match list, and
 * the likeliehoood function.
 */
class simple_likelihood_model_t final : public likelihood_model_t {
public:
  explicit simple_likelihood_model_t(const std::vector<match_t> &matches);
  ~simple_likelihood_model_t() override = default;

  double likelihood(const params_t &team_strs) const override {
    return std::exp(log_likelihood(team_strs));
  }

  double log_likelihood(const params_t &team_win_probs) const override;

  size_t param_count() const override {
    return (_team_count * (_team_count + 1)) / 2;
  }

  matrix_t generate_win_probs(const params_t &params) const override;

private:
  std::vector<std::vector<unsigned int>> _win_matrix;
  size_t                                 _team_count;
};

class poisson_likelihood_model_t final : public likelihood_model_t {
public:
  explicit poisson_likelihood_model_t(const std::vector<match_t> &matches) :
      _team_count{count_teams(matches)}, _matches{matches} {}
  ~poisson_likelihood_model_t() override = default;

  double likelihood(const params_t &team_strs) const override {
    return std::exp(log_likelihood(team_strs));
  }

  double log_likelihood(const params_t &team_strs) const override;

  size_t param_count() const override { return _team_count; }

  matrix_t generate_win_probs(const params_t &params) const override;

private:
  size_t               _team_count;
  std::vector<match_t> _matches;
};
#endif
