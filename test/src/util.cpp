#include <catch2/catch.hpp>
#include <limits>
#include <util.hpp>

TEST_CASE("skellam distribution calculators"
          "[skellam]") {
  SECTION("PMF") {
    CHECK(skellam_pmf(1, 1, 1) ==
          Approx(0.21526928924893757)
              .epsilon(std::numeric_limits<double>::epsilon() * 4));
    CHECK(skellam_pmf(1, 2, 1) == Approx(0.23846343848629697));
    CHECK(skellam_pmf(0, 2, 1) == Approx(0.21171208396194352));
    CHECK(skellam_pmf(0, 1, 3) == Approx(0.13112159537380774));
    CHECK(skellam_pmf(0, 1, 10) == Approx(0.0015111023190393729));
    CHECK(skellam_pmf(-1, 1, 10) == Approx(0.00438250270092616));
  }
  SECTION("CDF") {
    CHECK(skellam_cmf(1, 1, 1) ==
          Approx(0.8695234505257738)
              .epsilon(std::numeric_limits<double>::epsilon() * 4));
    CHECK(skellam_cmf(-1, 1, 1) == Approx(0.34574583872316267));
    CHECK(skellam_cmf(-1, 1, 10) == Approx(0.9979162474528441));
    CHECK(skellam_cmf(-1, 1, 10) == Approx(0.9979162474528441));
  }
}
