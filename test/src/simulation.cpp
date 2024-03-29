#include "simulation_node.hpp"
#include "tournament_factory.hpp"
#include "util.hpp"
#include <catch2/catch_all.hpp>
#include <numeric>

TEST_CASE("Simulation, basics", "[simulation]") {
  constexpr size_t team_count = 4;
  auto             t          = tournament_factory_simulation(team_count);
  auto             pmat       = uniform_matrix_factory(team_count);
  t.reset_win_probs(pmat);
  t.relabel_indicies();

  SECTION("Eval") {
    auto r   = t.eval(100000);
    auto sum = std::accumulate(r.begin(), r.end(), 0.0);
    CHECK(sum == Catch::Approx(1.0));
    CHECK(r[0] == Catch::Approx(0.25).margin(0.1));
    CHECK(r[1] == Catch::Approx(0.25).margin(0.1));
    CHECK(r[2] == Catch::Approx(0.25).margin(0.1));
    CHECK(r[3] == Catch::Approx(0.25).margin(0.1));
  }
  SECTION("Eval, smaller margins") {
    auto r   = t.eval(100000);
    auto sum = std::accumulate(r.begin(), r.end(), 0.0);
    CHECK(sum == Catch::Approx(1.0));
    CHECK(r[0] == Catch::Approx(0.25).margin(0.01));
    CHECK(r[1] == Catch::Approx(0.25).margin(0.01));
    CHECK(r[2] == Catch::Approx(0.25).margin(0.01));
    CHECK(r[3] == Catch::Approx(0.25).margin(0.01));
  }
}

TEST_CASE("Simulation, basics, larger", "[simulation]") {
  constexpr size_t team_count = 8;
  auto             t          = tournament_factory_simulation(team_count);
  auto             pmat       = uniform_matrix_factory(team_count);
  t.reset_win_probs(pmat);
  t.relabel_indicies();

  SECTION("Eval") {
    auto r   = t.eval(10000);
    auto sum = std::accumulate(r.begin(), r.end(), 0.0);
    CHECK(sum == Catch::Approx(1.0));
    CHECK(r[0] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[1] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[2] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[3] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[4] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[5] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[6] == Catch::Approx(0.125).margin(0.1));
    CHECK(r[7] == Catch::Approx(0.125).margin(0.1));
  }

  SECTION("Eval, smaller margins") {
    auto r   = t.eval(100000);
    auto sum = std::accumulate(r.begin(), r.end(), 0.0);
    CHECK(sum == Catch::Approx(1.0));
    CHECK(r[0] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[1] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[2] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[3] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[4] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[5] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[6] == Catch::Approx(0.125).margin(0.01));
    CHECK(r[7] == Catch::Approx(0.125).margin(0.01));
  }
}
