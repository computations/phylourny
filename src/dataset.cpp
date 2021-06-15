#include "dataset.hpp"

std::vector<match_t> generate_bootstrap(const std::vector<match_t> &matches,
                                        uint64_t seed) {
  std::vector<match_t> bs_matches;
  bs_matches.reserve(matches.size());
  std::mt19937 gen(seed);
  std::uniform_int_distribution<> boot_dis(0, matches.size() - 1);

  for (size_t i = 0; i < matches.size(); ++i) {
    auto &m = matches[boot_dis(gen)];
    bs_matches.push_back(m);
  }

  return bs_matches;
}

match_winner_t operator!(match_winner_t mw) {
  if (mw == match_winner_t::left)
    return match_winner_t::right;

  return match_winner_t::left;
}
