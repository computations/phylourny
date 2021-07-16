#ifndef DATASET_HPP
#define DATASET_HPP

#include "debug.h"
#include "match.hpp"
#include "tournament.hpp"
#include <random>
#include <vector>

typedef std::vector<double> params_t;

/**
 * A `dataset_t` is a class used to compute the likelihood of plausible pairwise
 * win matrices. The major concepts are the match list, and the likeliehoood
 * function.
 */
class dataset_t {
public:
  double likelihood(const params_t &team_win_probs) const;

  dataset_t(const std::vector<match_t> &matches);

  double log_likelihood(const params_t &p) const {
    return std::log(likelihood(p));
  }

private:
  inline size_t count_teams(const std::vector<match_t> &matches) const;
  std::vector<std::vector<unsigned int>> _win_matrix;
};

#endif
