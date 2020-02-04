#include <catch2/catch.hpp>
#include <debug.h>
#include <tournament.hpp>

TEST_CASE("tournament_node_t basic tests: 4 teamn single elim", "[tournament_t]"){
  std::shared_ptr<tournament_node_t> team_a{new tournament_node_t};
  std::shared_ptr<tournament_node_t> team_b{new tournament_node_t};
  std::shared_ptr<tournament_node_t> team_c{new tournament_node_t};
  std::shared_ptr<tournament_node_t> team_d{new tournament_node_t};

  std::shared_ptr<tournament_node_t> n1{
      new tournament_node_t{team_a, true, team_b, true}};
  std::shared_ptr<tournament_node_t> n2{
      new tournament_node_t{team_a, true, team_b, true}};

  std::shared_ptr<tournament_node_t> peak{
      new tournament_node_t{n1, true, n2, true}};

  SECTION("count nodes"){
    REQUIRE(peak->count()==4);
  }
}
