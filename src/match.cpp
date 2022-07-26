#include "match.hpp"
#include <cstddef>
#include <random>

/**
 * Generate a boostrapped version of the list of matches.
 */
auto generate_bootstrap(const std::vector<match_t> &matches,
                                        uint64_t                    seed) -> std::vector<match_t> {
  std::vector<match_t> bs_matches;
  bs_matches.reserve(matches.size());
  std::mt19937                          gen(seed);
  std::uniform_int_distribution<size_t> boot_dis(0, matches.size() - 1);

  for (size_t i = 0; i < matches.size(); ++i) {
    const auto &m = matches[static_cast<size_t>(boot_dis(gen))];
    bs_matches.push_back(m);
  }

  return bs_matches;
}

auto operator!(match_winner_t mw) -> match_winner_t {
  if (mw == match_winner_t::left) { return match_winner_t::right; }

  return match_winner_t::left;
}

auto count_teams(const std::vector<match_t> &matches) -> size_t {
  size_t cur_max = 0;
  for (auto m : matches) {
    cur_max = std::max(cur_max, m.l_team);
    cur_max = std::max(cur_max, m.r_team);
  }
  return cur_max + 1;
}
