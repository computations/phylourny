#ifndef MATCH_HPP
#define MATCH_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

enum struct match_winner_t {
  left = 1,
  right = 0,
};

struct match_t {
  size_t l_team;
  size_t r_team;
  match_winner_t winner;
};

match_winner_t operator!(match_winner_t mw);

std::vector<match_t> generate_bootstrap(const std::vector<match_t> &matches,
                                        uint64_t seed);
#endif // MATCH_HPP