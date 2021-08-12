#include "simulation_node.hpp"
#include "tournament_factory.hpp"
#include "util.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Simulation, basics", "[simulation]") {
  constexpr size_t team_count = 4;
  auto             t          = tournament_factory_simulation(team_count);
  auto             pmat       = uniform_matrix_factory(team_count);
  t.reset_win_probs(pmat);
  t.relabel_indicies();

  SECTION("Single eval") { auto r = t.eval(100000); }
}
