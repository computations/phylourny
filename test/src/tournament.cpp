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
  SECTION("sized 2") {
    auto t = tournament_factory(2);
    // Check for segfaults, and that the label map is well formed
    SECTION("Check the label map") {
      auto lm = t.label_map();
      CHECK(lm.size() == 2);
    }
    REQUIRE_THROWS(t.eval());
  }

  SECTION("sized 4") {
    auto t = tournament_factory(4);
    // Check for segfaults, and that the label map is well formed
    SECTION("Check the label map") {
      auto lm = t.label_map();
      CHECK(lm.size() == 4);
    }
    REQUIRE_THROWS(t.eval());
  }

  SECTION("sized 8") {
    auto t = tournament_factory(8);
    // Check for segfaults, and that the label map is well formed
    SECTION("Check the label map") {
      auto lm = t.label_map();
      CHECK(lm.size() == 8);
    }
    REQUIRE_THROWS(t.eval());
  }

  SECTION("sized 16") {
    auto t = tournament_factory(16);
    // Check for segfaults, and that the label map is well formed
    SECTION("Check the label map") {
      auto lm = t.label_map();
      CHECK(lm.size() == 16);
    }
    REQUIRE_THROWS(t.eval());
  }
  SECTION("sized 15, not a power of 2") {
    REQUIRE_THROWS(tournament_factory(15));
  }
}

TEST_CASE("tournament_t larger cases", "[tournament_t]") {
  SECTION("sized 16") {
    size_t tsize = 16;
    auto t = tournament_factory(tsize);
    SECTION("Uniform win probs") {
      auto m = uniform_matrix_factory(tsize);
      t.reset_win_probs(m);
      auto r = t.eval();
      double sum = 0.0;
      for (auto f : r) {
        CHECK(f == Approx(1.0 / static_cast<double>(tsize)));
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }
    SECTION("Random win probs") {
      auto m = random_matrix_factory(tsize, rand());
      t.reset_win_probs(m);
      auto r = t.eval();
      double sum = 0.0;
      for (auto f : r) {
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }
  }
  SECTION("sized 32") {
    size_t tsize = 32;
    auto t = tournament_factory(tsize);
    SECTION("Uniform win probs") {
      auto m = uniform_matrix_factory(tsize);
      t.reset_win_probs(m);
      auto r = t.eval();
      double sum = 0.0;
      for (auto f : r) {
        CHECK(f == Approx(1.0 / static_cast<double>(tsize)));
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }
    SECTION("Random win probs") {
      auto m = random_matrix_factory(tsize, rand());
      t.reset_win_probs(m);
      auto r = t.eval();
      double sum = 0.0;
      for (auto f : r) {
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }
  }
}

TEST_CASE("tournament_t, unbalanced", "[tournament_t]") {
  SECTION("left sized 16, right sized 8") {
    size_t left_size = 16, right_size = 8;
    size_t total_size = left_size + right_size;
    auto t1 = tournament_factory(left_size, right_size);

    CHECK(t1.tip_count() == (total_size));

    SECTION("uniform rate matrix") {
      auto m1 = uniform_matrix_factory((total_size));
      t1.reset_win_probs(m1);
      auto r1 = t1.eval();
      double sum = 0.0;
      for (auto f : r1) {
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }

    SECTION("random rate matrix") {
      auto m1 = random_matrix_factory((total_size), rand());
      t1.reset_win_probs(m1);
      auto r1 = t1.eval();
      double sum = 0.0;
      for (auto f : r1) {
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }
  }

  SECTION("left sized 8, right size 16") {
    size_t left_size = 8, right_size = 16;
    size_t total_size = left_size + right_size;
    auto t1 = tournament_factory(left_size, right_size);

    CHECK(t1.tip_count() == (total_size));

    SECTION("uniform rate matrix") {
      auto m1 = uniform_matrix_factory((total_size));
      t1.reset_win_probs(m1);
      auto r1 = t1.eval();
      double sum = 0.0;
      for (auto f : r1) {
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }

    SECTION("random rate matrix") {
      auto m1 = random_matrix_factory((total_size), rand());
      t1.reset_win_probs(m1);
      auto r1 = t1.eval();
      double sum = 0.0;
      for (auto f : r1) {
        sum += f;
      }
      CHECK(sum == Approx(1.0));
    }
  }
}

TEST_CASE("4 team tournament with losers bracket") {
  std::shared_ptr<tournament_node_t> n1{new tournament_node_t{"a"}};
  std::shared_ptr<tournament_node_t> n2{new tournament_node_t{"b"}};
  std::shared_ptr<tournament_node_t> n3{new tournament_node_t{"c"}};
  std::shared_ptr<tournament_node_t> n4{new tournament_node_t{"d"}};

  std::shared_ptr<tournament_node_t> w1{
      new tournament_node_t{
          (n1),
          tournament_edge_t::edge_type_t::win,
          (n2),
          tournament_edge_t::edge_type_t::win,
      },
  };

  std::shared_ptr<tournament_node_t> w2{
      new tournament_node_t{
          (n3),
          tournament_edge_t::edge_type_t::win,
          (n4),
          tournament_edge_t::edge_type_t::win,
      },
  };

  std::shared_ptr<tournament_node_t> w3{
      new tournament_node_t{
          (w1),
          tournament_edge_t::edge_type_t::win,
          (w2),
          tournament_edge_t::edge_type_t::win,
      },
  };

  std::shared_ptr<tournament_node_t> l3{
      new tournament_node_t{
          (w1),
          tournament_edge_t::edge_type_t::loss,
          (w2),
          tournament_edge_t::edge_type_t::loss,
      },
  };

  std::shared_ptr<tournament_node_t> l4{
      new tournament_node_t{
          w3,
          tournament_edge_t::edge_type_t::loss,
          l3,
          tournament_edge_t::edge_type_t::win,
      },
  };

  tournament_t t{tournament_node_t{
      l4,
      w3,
  }};

  t.relabel_indicies();
  size_t tip_count = t.tip_count();
  auto m = uniform_matrix_factory(tip_count);
  t.reset_win_probs(m);

  auto r = t.eval();
  double sum = 0.0;
  for (auto f : r) {
    sum += f;
    CHECK(f == Approx(1.0 / tip_count));
  }
  CHECK(sum == Approx(1.0));
}

TEST_CASE("Best tests", "[bestof_n]") {
  SECTION("Best ofs with equal probs") {
    CHECK(bestof_n(0.5, 0.5, 1) == 0.5);
    CHECK(bestof_n(0.5, 0.5, 3) == 0.5);
    CHECK(bestof_n(0.5, 0.5, 5) == 0.5);
  }

  SECTION("Best ofs with 0.25, 0.75 probs") {
    CHECK(bestof_n(0.25, 0.75, 1) == Approx(0.25));
    CHECK(bestof_n(0.25, 0.75, 3) == Approx(0.15625));
    CHECK(bestof_n(0.25, 0.75, 5) == Approx(0.103515625));
  }

  SECTION("Best ofs with 0.25, 0.75 probs") {
    CHECK(bestof_n(0.75, 0.25, 1) == Approx(0.75));
    CHECK(bestof_n(0.75, 0.25, 3) == Approx(0.84375));
    CHECK(bestof_n(0.75, 0.25, 5) == Approx(0.896484375));
  }

  SECTION("Grid search for inverse complimentarity") {
    for (size_t i = 1; i < 16; ++i) {
      for (size_t j = 1; j < 16; ++j) {
        double p = static_cast<double>(i) / static_cast<double>(j);
        for (size_t n = 1; n < 16; ++n) {
          CHECK(bestof_n(p, 1 - p, n) == Approx(1 - bestof_n(1 - p, p, n)));
        }
      }
    }
  }

  SECTION("Fuzzed tests") {
    for (size_t i = 0; i < 1e4; ++i) {
      double p = rand();
      for (size_t n = 1; n < 16; ++n) {
        CHECK(bestof_n(p, 1 - p, n) == Approx(1 - bestof_n(1 - p, p, n)));
      }
    }
  }
}
