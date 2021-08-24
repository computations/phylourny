#ifndef DATASET_HPP
#define DATASET_HPP

#include "debug.h"
#include "match.hpp"
#include "tournament.hpp"
#include <random>
#include <vector>

typedef std::vector<double> params_t;

class likelihood_model_t {
public:
  virtual double likelihood(const params_t &) const = 0;

  double log_likelihood(const params_t &p) const {
    return std::log(likelihood(p));
  }

  virtual ~likelihood_model_t() = default;
};

/**
 * A `simple_likelihood_model_t` is a class used to compute the likelihood of
 * plausible pairwise win matrices. The major concepts are the match list, and
 * the likeliehoood function.
 */
class simple_likelihood_model_t final : public likelihood_model_t {
public:
  simple_likelihood_model_t(const std::vector<match_t> &matches);

  virtual double likelihood(const params_t &team_win_probs) const;

  virtual ~simple_likelihood_model_t() = default;

private:
  inline size_t count_teams(const std::vector<match_t> &matches) const;
  std::vector<std::vector<unsigned int>> _win_matrix;
};
#endif
