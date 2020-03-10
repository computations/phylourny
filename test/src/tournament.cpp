#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <debug.h>
#include <tournament.hpp>

TEST_CASE("tournament_node_t basic tests: 4 teamn single elim",
          "[tournament_node_t]") {
  std::unique_ptr<tournament_node_t> team_a{new tournament_node_t};
  std::unique_ptr<tournament_node_t> team_b{new tournament_node_t};
  std::unique_ptr<tournament_node_t> team_c{new tournament_node_t};
  std::unique_ptr<tournament_node_t> team_d{new tournament_node_t};

  std::unique_ptr<tournament_node_t> n1{
      new tournament_node_t{std::move(team_a), std::move(team_b)}};

  std::unique_ptr<tournament_node_t> n2{
      new tournament_node_t{std::move(team_c), std::move(team_d)}};

  std::unique_ptr<tournament_node_t> peak{
      new tournament_node_t{std::move(n1), std::move(n2)}};

  SECTION("Count nodes") { CHECK(peak->tip_count() == 4); }
  SECTION("Is tip") { CHECK(!peak->is_tip()); }
  SECTION("Relabel nodes and map") {
    SECTION("Indexing") {
      auto result = peak->relabel_indicies(0);
      CHECK(result == peak->tip_count());
    }
    SECTION("Relabeling") {
      auto ind_result = peak->relabel_indicies(0);

      std::vector<std::string> labels{"a", "b", "c", "d"};
      REQUIRE_NOTHROW(peak->relabel_tips(labels));
      std::vector<std::pair<std::string, size_t>> lm;
      peak->label_map(lm);

      CHECK(lm.size() == peak->tip_count());
      CHECK(lm.size() == ind_result);
      CHECK(lm[0].first == "a");
      CHECK(lm[0].second == 0);
      CHECK(lm[1].first == "b");
      CHECK(lm[1].second == 1);
      CHECK(lm[2].first == "c");
      CHECK(lm[2].second == 2);
      CHECK(lm[3].first == "d");
      CHECK(lm[3].second == 3);
    }
  }
}

TEST_CASE("tournament_t default case", "[tournament_t]") {
  tournament_t tournament;

  SECTION("count nodes") { CHECK(tournament.tip_count() == 2); }

  SECTION("Tip Labels and Indicies") {

    SECTION("Correctly sized vector") {
      std::vector<std::string> labels{"a", "b"};
      REQUIRE_NOTHROW(tournament.relabel_tips(labels));

      auto lm = tournament.label_map();
      REQUIRE(lm.size() == 2);
      CHECK(lm[0].first == "a");
      CHECK(lm[0].second == 0);
      CHECK(lm[1].first == "b");
      CHECK(lm[1].second == 1);
    }

    SECTION("Incorrectly sized vector") {
      std::vector<std::string> labels{"a"};
      REQUIRE_THROWS(tournament.relabel_tips(labels));
    }
  }

  SECTION("reset win probs") {
    const matrix_t wps1{{0.0, 0.5}, {0.5, 0.0}};
    REQUIRE_NOTHROW(tournament.reset_win_probs(wps1));

    const matrix_t wps2{{0.0, 0.5, 0.5}, {0.5, 0.0, 0.5}, {0.5, 0.5, 0.0}};
    REQUIRE_THROWS(tournament.reset_win_probs(wps2));
  }
}

TEST_CASE("tournament_factory", "[tournament_t]") {
  SECTION("sized 16"){
    auto t = tournament_factory(16);
    // Check for segfaults, and that the label map is well formed
    SECTION("Check the label map") {
      auto lm = t.label_map();
      CHECK(lm.size() == 16);
    }
    REQUIRE_THROWS(t.eval());
  }
  SECTION("sized 15, not a power of 2"){
    REQUIRE_THROWS(tournament_factory(15));
  }
}

TEST_CASE("tournament_t larger cases", "[tournament_t]") {
  SECTION("sized 16") { 
    size_t tsize = 16;
    auto t = tournament_factory(tsize); 
    auto m = uniform_matrix_factory(tsize);
    t.reset_win_probs(m);
    auto r = t.eval();
    double sum = 0.0;
    for (auto f : r){
      CHECK(f == Approx(1.0/static_cast<double>(tsize)));
      sum += f;
    }
    CHECK(sum == Approx(1.0));
    BENCHMARK("eval()"){
      return t.eval();
    };
  }
  SECTION("sized 32"){
    size_t tsize = 32;
    auto t = tournament_factory(tsize); 
    auto m = uniform_matrix_factory(tsize);
    t.reset_win_probs(m);
    auto r = t.eval();
    double sum = 0.0;
    for (auto f : r){
      CHECK(f == Approx(1.0/static_cast<double>(tsize)));
      sum += f;
    }
    CHECK(sum == Approx(1.0));
    BENCHMARK("eval()"){
      return t.eval();
    };
  }
}
