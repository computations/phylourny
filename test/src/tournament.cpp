#include <Eigen/Dense>
#include <catch2/catch.hpp>
#include <debug.h>
#include <tournament.hpp>

TEST_CASE("tournament_node_t basic tests: 4 teamn single elim",
          "[tournament_node_t]") {
  std::shared_ptr<tournament_node_t> team_a{new tournament_node_t};
  std::shared_ptr<tournament_node_t> team_b{new tournament_node_t};
  std::shared_ptr<tournament_node_t> team_c{new tournament_node_t};
  std::shared_ptr<tournament_node_t> team_d{new tournament_node_t};

  std::shared_ptr<tournament_node_t> n1{
      new tournament_node_t{team_a, tournament_edge_t::edge_type_t::win, team_b,
                            tournament_edge_t::edge_type_t::win}};
  std::shared_ptr<tournament_node_t> n2{
      new tournament_node_t{team_a, tournament_edge_t::edge_type_t::win, team_b,
                            tournament_edge_t::edge_type_t::win}};

  std::shared_ptr<tournament_node_t> peak{
      new tournament_node_t{n1, tournament_edge_t::edge_type_t::win, n2,
                            tournament_edge_t::edge_type_t::win}};

  SECTION("count nodes") { CHECK(peak->tip_count() == 4); }
}

TEST_CASE("touranment_t default case", "[tournament_t]") {
  tournament_t tournament;

  SECTION("count nodes") { CHECK(tournament.tip_count() == 2); }

  SECTION("reset win probs") {
    const Eigen::MatrixXd wps1(2, 2);
    REQUIRE_NOTHROW(tournament.reset_win_probs(wps1));

    const Eigen::MatrixXd wps2(3, 3);
    REQUIRE_THROWS(tournament.reset_win_probs(wps2));
  }
}
