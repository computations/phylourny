#ifndef MATCH_HPP
#define MATCH_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

enum struct match_winner_t {
  left  = 1,
  right = 0,
};

struct match_t {
  size_t         l_team;
  size_t         r_team;
  size_t         l_goals;
  size_t         r_goals;
  match_winner_t winner;
};

auto count_teams(const std::vector<match_t> &matches) -> size_t;

auto operator!(match_winner_t mw) -> match_winner_t;

auto generate_bootstrap(const std::vector<match_t> &matches, uint64_t seed)
    -> std::vector<match_t>;
#endif
