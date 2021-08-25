#ifndef MATCH_HPP
#define MATCH_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

enum struct match_winner_t {
  left  = 1,
  right = 0,
  // tie,
};

struct match_t {
  size_t         l_team;
  size_t         r_team;
  size_t         l_goals;
  size_t         r_goals;
  match_winner_t winner;
};

size_t count_teams(const std::vector<match_t> &matches);

match_winner_t operator!(match_winner_t mw);

std::vector<match_t> generate_bootstrap(const std::vector<match_t> &matches,
                                        uint64_t                    seed);
#endif // MATCH_HPP
