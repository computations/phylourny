#include <catch2/catch.hpp>
#include <dataset.hpp>
#include <debug.h>

TEST_CASE("dataset_t simple cases", "[dataset_t]") {
  std::vector<match_t> matches;
  matches.push_back({0, 1, match_winner_t::left});
  matches.push_back({0, 1, match_winner_t::right});
  SECTION("default constructor") {
    dataset_t ds(matches);
    SECTION("likelihood") {
      params_t params{0.5, 0.5};
      double lh = ds.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Approx(0.5));
    }
  }
}

TEST_CASE("dataset_t slightly more complex cases", "[dataset_t]") {
  std::vector<match_t> matches;
  matches.push_back({0, 1, match_winner_t::left});
  matches.push_back({0, 1, match_winner_t::right});
  matches.push_back({0, 1, match_winner_t::left});
  matches.push_back({0, 1, match_winner_t::left});
  SECTION("default constructor") {
    dataset_t ds(matches);
    SECTION("likelihood, uniform params") {
      params_t params{0.5, 0.5};
      double lh = ds.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Approx(0.25));
    }

    SECTION("likelihood, team 0 superior params") {
      params_t params{0.25, 0.75};
      double lh = ds.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Approx(0.421875));
    }

    SECTION("likelihood, team 1 superior params") {
      params_t params{0.75, 0.25};
      double lh = ds.likelihood(params);
      CHECK(!std::isnan(lh));
      CHECK(lh == Approx(0.046875));
    }
  }
}
