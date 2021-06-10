#ifndef DATASET_HPP
#define DATASET_HPP

#include "debug.h"
#include "tournament.hpp"
#include <random>
#include <vector>

typedef std::vector<double> params_t;

struct match_t {
  size_t l_team;
  size_t r_team;
  bool l_win;
};

std::vector<match_t> generate_bootstrap(const std::vector<match_t> &matches,
                                        uint64_t seed);

class dataset_t {
public:
  dataset_t(const std::vector<match_t> &matches) {
    size_t team_count = count_teams(matches);
    _win_matrix.reserve(team_count);
    for (size_t i = 0; i < team_count; ++i) {
      _win_matrix.emplace_back(team_count);
    }

    for (auto &m : matches) {
      _win_matrix[m.r_team][m.l_team] += 1 * static_cast<unsigned int>(m.l_win);
      _win_matrix[m.l_team][m.r_team] +=
          1 * static_cast<unsigned int>(!m.l_win);
    }
  }

  double likelihood(const params_t &team_win_probs) const {
    double lh = 1.0;
    debug_print(EMIT_LEVEL_DEBUG, "team_win_probs: %s",
                to_string(team_win_probs).c_str());
    for (size_t i = 0; i < _win_matrix.size(); ++i) {
      for (size_t j = i + 1; j < _win_matrix.size(); ++j) {
        double l_wp =
            team_win_probs[i] / (team_win_probs[i] + team_win_probs[j]);
        double r_wp = 1 - l_wp;
        debug_print(EMIT_LEVEL_DEBUG,
                    "twp[i]: %f, twp[j]: %f, l_wp: %f, r_wp: %f",
                    team_win_probs[i], team_win_probs[j], l_wp, r_wp);
        lh *= int_pow(l_wp, _win_matrix[i][j]) *
              int_pow(r_wp, _win_matrix[j][i]) *
              combinations(_win_matrix[i][j] + _win_matrix[j][i],
                           _win_matrix[i][j]);
      }
    }
    return lh;
  }

private:
  inline size_t count_teams(const std::vector<match_t> &matches) {
    size_t cur_max = 0;
    for (auto m : matches) {
      cur_max = std::max(cur_max, m.l_team);
      cur_max = std::max(cur_max, m.r_team);
    }
    return cur_max + 1;
  }
  std::vector<std::vector<unsigned int>> _win_matrix;
};

#endif