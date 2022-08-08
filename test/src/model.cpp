#include <catch2/catch_all.hpp>
#include <debug.h>
#include <model.hpp>

TEST_CASE("simple cases",
          "[simple_likelihood_model_t][poisson_likelihood_model_t]") {
  std::vector<match_t> matches;
  matches.push_back({0, 1, 0, 1, match_winner_t::left});
  matches.push_back({0, 1, 1, 0, match_winner_t::right});
  SECTION("Simple Likelihood model") {
    SECTION("default constructor") {
      simple_likelihood_model_t lhm(matches);
      SECTION("likelihood") {
        params_t params{0.5, 0.5};
        double   lh = lhm.likelihood(params);
        CHECK(!std::isnan(lh));
        CHECK(lh == Catch::Approx(0.5));
      }
    }
  }
  SECTION("Poisson Likelihood model") {
    SECTION("default constructor") {
      poisson_likelihood_model_t lhm(matches);
      SECTION("likelihood") {
        params_t params{0.0};
        double   lh = lhm.likelihood(params);
        CHECK(!std::isnan(lh));
        CHECK(lh == Catch::Approx(0.01831563888873418));
      }
      SECTION("Win probs") {
        params_t params{0.0};
        auto     wp = lhm.generate_win_probs(params, {0, 1});
        CHECK(wp[0][1] == Catch::Approx(0.5));
        CHECK(wp[1][0] == Catch::Approx(0.5));
      }
    }
  }
}

TEST_CASE("simple_likelihood_model_t slightly more complex cases",
          "[simple_likelihood_model_t]") {
  std::vector<match_t> matches;
  matches.push_back({0, 1, 0, 0, match_winner_t::left});
  matches.push_back({0, 1, 0, 0, match_winner_t::right});
  matches.push_back({0, 1, 0, 0, match_winner_t::left});
  matches.push_back({0, 1, 0, 0, match_winner_t::left});
  SECTION("default constructor") {
    simple_likelihood_model_t lhm(matches);
    SECTION("likelihood, uniform params") {
      params_t params{0.5, 0.5};
      double   lh = lhm.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Catch::Approx(0.25));
    }

    SECTION("likelihood, team 0 superior params") {
      params_t params{0.25, 0.75};
      double   lh = lhm.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Catch::Approx(0.421875));
    }

    SECTION("likelihood, team 1 superior params") {
      params_t params{0.75, 0.25};
      double   lh = lhm.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Catch::Approx(0.046875));
    }
  }
}
