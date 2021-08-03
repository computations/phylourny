#include "tournament_factory.hpp"
#include "util.hpp"
#include <algorithm>
#include <catch2/catch.hpp>
#include <cstdio>
#include <debug.h>
#include <memory>
#include <numeric>
#include <tournament.hpp>

TEST_CASE("single node basic tests", "[single_node]") {
  SECTION("Default Constructor") { single_node_t s; }

  SECTION("Unique pointer construtor") {
    std::unique_ptr<tournament_node_t> team_a{new single_node_t};
  }

  SECTION("Making a simple bracket") {
    std::shared_ptr<single_node_t> team_a{new single_node_t};
    std::shared_ptr<single_node_t> team_b{new single_node_t};

    std::unique_ptr<tournament_node_t> n1{
        new single_node_t{std::move(team_a), std::move(team_b)}};
  }

  SECTION("Making a simple tournament") {
    std::shared_ptr<single_node_t> team_a{new single_node_t};
    std::shared_ptr<single_node_t> team_b{new single_node_t};

    std::unique_ptr<single_node_t> n1 =
        std::make_unique<single_node_t>(team_a, team_b);

    tournament_t t(std::move(n1));
  }

  SECTION("Testing ticks and evals") {
    std::shared_ptr<single_node_t> team_a{new single_node_t};
    std::shared_ptr<single_node_t> team_b{new single_node_t};

    std::unique_ptr<single_node_t> n1{
        new single_node_t{std::move(team_a), std::move(team_b)}};

    n1->relabel_indicies(0);
    n1->set_tip_bitset(2);
    n1->init_assigned_teams();

    CHECK(n1->is_cherry());

    SECTION("Testing ticks") {
      CHECK(n1->winner() == 0);

      auto tr = n1->tick();
      CHECK(n1->winner() == 1);
      CHECK(tr == tick_result_t::success);

      tr = n1->tick();
      CHECK(n1->winner() == 0);
      CHECK(tr == tick_result_t::finished);
    }

    SECTION("Testing valid") {
      CHECK(n1->valid());
      n1->tick();
      CHECK(n1->valid());
    }

    SECTION("Testing eval, uniform matrix") {
      auto pmat = uniform_matrix_factory(2);
      auto r    = n1->eval(pmat, 2);

      auto sum = std::accumulate(r.begin(), r.end(), 0.0);
      CHECK(sum == Approx(1.0));
      CHECK(r[0] == Approx(0.5));
      CHECK(r[1] == Approx(0.5));
    }

    SECTION("Testing eval, random matrix") {
      auto pmat = random_matrix_factory(2, rand());
      auto r    = n1->eval(pmat, 2);

      auto sum = std::accumulate(r.begin(), r.end(), 0.0);
      CHECK(sum == Approx(1.0));
    }
  }
}

TEST_CASE("4 Team tournament tests", "[single_node]") {
  constexpr size_t team_count = 4;
  auto             t          = tournament_node_factory_single(team_count);
  t->relabel_indicies(0);
  t->set_tip_bitset(team_count);
  t->init_assigned_teams();

  SECTION("Testing ticks") {
    SECTION("One pass") {
      for (size_t i = 0; i < team_count; i++) {
        CHECK(t->winner() == i);
        t->tick();
      }
      CHECK(t->winner() == 0);
    }
    SECTION("Full pass") {
      size_t tick_count = 0;
      for (auto tr = t->tick(); tr != tick_result_t::finished; tr = t->tick()) {
        tick_count++;
        REQUIRE(tick_count <= ((1 << team_count) - 1));
      }
      CHECK(tick_count == (1 << team_count) - 1);
    }
  }

  SECTION("Testing valid") {

    bool valids[] = {true,
                     false,
                     true,
                     false,
                     true,
                     false,
                     false,
                     true,
                     false,
                     true,
                     true,
                     false,
                     false,
                     true,
                     false,
                     true};

    for (size_t i = 0; i < (1 << team_count); i++) {
      CHECK(t->valid() == valids[i]);
      t->tick();
    }
  }
  SECTION("Testing Eval") {
    SECTION("Uniform matrix") {
      auto pmat = uniform_matrix_factory(team_count);
      auto r    = t->eval(pmat, team_count);

      auto sum = std::accumulate(r.begin(), r.end(), 0.0);
      CHECK(sum == Approx(1.0));
      CHECK(r[0] == Approx(1.0 / team_count));
      CHECK(r[1] == Approx(1.0 / team_count));
      CHECK(r[2] == Approx(1.0 / team_count));
      CHECK(r[3] == Approx(1.0 / team_count));
    }
    SECTION("Random matrix") {
      auto pmat = random_matrix_factory(team_count, rand());
      auto r    = t->eval(pmat, team_count);

      auto sum = std::accumulate(r.begin(), r.end(), 0.0);
      CHECK(sum == Approx(1.0));
    }
  }
}

TEST_CASE("Double Elim Tournament", "[single_node]") {
  constexpr size_t team_count = 4;

  std::shared_ptr<single_node_t> n1{new single_node_t{"a"}};
  std::shared_ptr<single_node_t> n2{new single_node_t{"b"}};
  std::shared_ptr<single_node_t> n3{new single_node_t{"c"}};
  std::shared_ptr<single_node_t> n4{new single_node_t{"d"}};

  std::shared_ptr<single_node_t> w1{
      new single_node_t{
          (n1),
          tournament_edge_t::edge_type_e::win,
          (n2),
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> w2{
      new single_node_t{
          (n3),
          tournament_edge_t::edge_type_e::win,
          (n4),
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> w3{
      new single_node_t{
          (w1),
          tournament_edge_t::edge_type_e::win,
          (w2),
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> l3{
      new single_node_t{
          (w1),
          tournament_edge_t::edge_type_e::loss,
          (w2),
          tournament_edge_t::edge_type_e::loss,
      },
  };

  std::shared_ptr<single_node_t> l4{
      new single_node_t{
          w3,
          tournament_edge_t::edge_type_e::loss,
          l3,
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> t{new single_node_t{
      l4,
      w3,
  }};

  t->relabel_indicies(0);
  t->set_tip_bitset(team_count);
  t->init_assigned_teams();

  constexpr size_t node_count = 6;

  SECTION("Testing ticks") {
    SECTION("One pass") {
      for (size_t i = 0; i < team_count; i++) {
        CHECK(t->winner() == i);
        t->tick();
      }
      CHECK(t->winner() == 0);
    }

    SECTION("Full pass") {
      size_t tick_count = 0;
      for (auto tr = t->tick(); tr != tick_result_t::finished; tr = t->tick()) {
        tick_count++;
        REQUIRE(tick_count < (1 << node_count + team_count));
      }
      CHECK(tick_count == (1 << node_count + team_count) - 1);
    }
  }

  SECTION("Testing valid") {
    size_t valid_count = 0;

    for (auto tr = t->tick(); tr != tick_result_t::finished; tr = t->tick()) {
      if (t->valid()) { valid_count++; };
    }
    CHECK(valid_count == 64);
  }

  SECTION("Testing eval") {
    SECTION("Uniform matrix") {
      t->assign_internal_labels(0);

      auto pmat = uniform_matrix_factory(team_count);
      auto r    = t->eval(pmat, team_count);

      double sum = std::accumulate(r.begin(), r.end(), 0.0);
      CHECK(sum == Approx(1.0));
      CHECK(r[0] == Approx(1.0 / team_count));
      CHECK(r[1] == Approx(1.0 / team_count));
      CHECK(r[2] == Approx(1.0 / team_count));
      CHECK(r[3] == Approx(1.0 / team_count));
    }
  }
}

TEST_CASE("Double elim tournament with tournament_t",
          "[single_node][tournament]") {
  constexpr size_t team_count = 4;

  std::shared_ptr<single_node_t> n1{new single_node_t{"a"}};
  std::shared_ptr<single_node_t> n2{new single_node_t{"b"}};
  std::shared_ptr<single_node_t> n3{new single_node_t{"c"}};
  std::shared_ptr<single_node_t> n4{new single_node_t{"d"}};

  std::shared_ptr<single_node_t> w1{
      new single_node_t{
          (n1),
          tournament_edge_t::edge_type_e::win,
          (n2),
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> w2{
      new single_node_t{
          (n3),
          tournament_edge_t::edge_type_e::win,
          (n4),
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> w3{
      new single_node_t{
          (w1),
          tournament_edge_t::edge_type_e::win,
          (w2),
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::shared_ptr<single_node_t> l3{
      new single_node_t{
          (w1),
          tournament_edge_t::edge_type_e::loss,
          (w2),
          tournament_edge_t::edge_type_e::loss,
      },
  };

  std::shared_ptr<single_node_t> l4{
      new single_node_t{
          w3,
          tournament_edge_t::edge_type_e::loss,
          l3,
          tournament_edge_t::edge_type_e::win,
      },
  };

  std::unique_ptr<single_node_t> t{new single_node_t{
      l4,
      w3,
  }};

  tournament_t tourny(std::move(t));

  SECTION("Eval") {
    SECTION("Uniform matrix") {
      auto pmat = uniform_matrix_factory(team_count);
      tourny.reset_win_probs(pmat);

      auto   r   = tourny.eval();
      double sum = std::accumulate(r.begin(), r.end(), 0.0);

      CHECK(sum == Approx(1.0));
      CHECK(r[0] == Approx(1.0 / team_count));
      CHECK(r[1] == Approx(1.0 / team_count));
      CHECK(r[2] == Approx(1.0 / team_count));
      CHECK(r[3] == Approx(1.0 / team_count));
    }
  }
}
