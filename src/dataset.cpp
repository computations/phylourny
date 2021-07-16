#include "dataset.hpp"

/**
 * Construct a dataset class given a list of matches. The matches are of a
 * `match_t` type, and encode a match played between the two teams.
 */
dataset_t::dataset_t(const std::vector<match_t> &matches) {
  size_t team_count = count_teams(matches);
  _win_matrix.reserve(team_count);
  for (size_t i = 0; i < team_count; ++i) {
    _win_matrix.emplace_back(team_count);
  }

  for (auto &m : matches) {
    _win_matrix[m.r_team][m.l_team] += 1 * static_cast<unsigned int>(m.winner);
    _win_matrix[m.l_team][m.r_team] += 1 * static_cast<unsigned int>(!m.winner);
  }
}

/**
 * Using the list of matches, this function computes the likelhood of the
 * proposed win probabilities.
 */
double dataset_t::likelihood(const params_t &team_win_probs) const {
  double lh = 1.0;
  debug_print(EMIT_LEVEL_DEBUG, "team_win_probs: %s",
              to_string(team_win_probs).c_str());
  debug_print(EMIT_LEVEL_DEBUG, "win matrix size: %lu", _win_matrix.size());
  for (size_t i = 0; i < _win_matrix.size(); ++i) {
    for (size_t j = i + 1; j < _win_matrix.size(); ++j) {
      double l_wp = team_win_probs[i] / (team_win_probs[i] + team_win_probs[j]);
      double r_wp = 1 - l_wp;
      debug_print(EMIT_LEVEL_DEBUG,
                  "twp[i]: %f, twp[j]: %f, l_wp: %f, r_wp: %f i: %lu, j: %lu",
                  team_win_probs[i], team_win_probs[j], l_wp, r_wp, i, j);
      lh *= int_pow(l_wp, _win_matrix[i][j]) *
            int_pow(r_wp, _win_matrix[j][i]) *
            combinations(_win_matrix[i][j] + _win_matrix[j][i],
                         _win_matrix[i][j]);
    }
  }
  debug_print(EMIT_LEVEL_DEBUG, "computed lh: %f", lh);
  return lh;
}

inline size_t
dataset_t::count_teams(const std::vector<match_t> &matches) const {
  size_t cur_max = 0;
  for (auto m : matches) {
    cur_max = std::max(cur_max, m.l_team);
    cur_max = std::max(cur_max, m.r_team);
  }
  debug_print(EMIT_LEVEL_DEBUG, "found %lu teams", cur_max);
  return cur_max + 1;
}
