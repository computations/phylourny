#include "match.hpp"
#include <cstddef>
#include <random>

/**
 * Generate a boostrapped version of the list of matches.
 */
std::vector<match_t> generate_bootstrap(const std::vector<match_t> &matches,
                                        uint64_t                    seed) {
  std::vector<match_t> bs_matches;
  bs_matches.reserve(matches.size());
  std::mt19937                          gen(seed);
  std::uniform_int_distribution<size_t> boot_dis(0, matches.size() - 1);

  for (size_t i = 0; i < matches.size(); ++i) {
    auto &m = matches[static_cast<size_t>(boot_dis(gen))];
    bs_matches.push_back(m);
  }

  return bs_matches;
}

match_winner_t operator!(match_winner_t mw) {
  // if (mw == match_winner_t::tie) { return mw; }
  if (mw == match_winner_t::left) { return match_winner_t::right; }

  return match_winner_t::left;
}

size_t count_teams(const std::vector<match_t> &matches) {
  size_t cur_max = 0;
  for (auto m : matches) {
    cur_max = std::max(cur_max, m.l_team);
    cur_max = std::max(cur_max, m.r_team);
  }
  return cur_max + 1;
}
