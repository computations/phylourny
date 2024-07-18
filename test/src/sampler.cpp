#include <algorithm>
#include <catch2/catch_all.hpp>
#include <cmath>
#include <debug.h>
#include <math.h>
#include <mcmc.hpp>
#include <memory>
#include <numeric>
#include <sampler.hpp>
#include <tournament_factory.hpp>
#include <util.hpp>

TEST_CASE("sampler_t simple case", "[sampler_t]") {
  std::vector<match_t> matches;
  matches.push_back({0, 1, 1, 0, match_winner_t::left});
  matches.push_back({0, 1, 0, 1, match_winner_t::right});
  matches.push_back({0, 1, 0, 1, match_winner_t::right});

  std::vector<std::string> teams{"a", "b"};
  team_name_map_t          team_name_map{
               {"a", 0},
               {"b", 1},
  };

  SECTION("Simple likelihood model") {
    SECTION("constructor") {
      auto      t = tournament_factory(2);
      sampler_t s{std::make_unique<simple_likelihood_model_t>(
                      simple_likelihood_model_t(matches)),
                  std::move(t)};
      SECTION("Running the chain") {
        results_t r{teams, team_name_map};
        s.run_chain(r,
                    100,
                    0,
                    Catch::rngSeed(),
                    update_win_probs_uniform,
                    uniform_prior);
        CHECK(r.sample_count() > 0);
        CHECK(r.sample_count() <= 100);
      }
    }
  }
  SECTION("Poisson likelihood model") {
    SECTION("constructor") {
      auto      t = tournament_factory(2);
      sampler_t s{std::make_unique<poisson_likelihood_model_t>(
                      poisson_likelihood_model_t(matches)),
                  std::move(t)};
      SECTION("Running the chain") {
        results_t r{teams, team_name_map};
        s.run_chain(r,
                    100,
                    0,
                    Catch::rngSeed(),
                    update_poission_model_factory(0.1),
                    uniform_prior);
        CHECK(r.sample_count() > 0);
        CHECK(r.sample_count() <= 100);
      }
    }
  }
}

TEST_CASE("beta distribution", "[beta_distribution]") {
  std::mt19937_64 gen(static_cast<uint64_t>(rand()));
  SECTION("uniform") {
    double                    a = 1.0, b = 1.0;
    beta_distribution<double> beta_dis(a, b);
    std::vector<double>       samples;
    for (size_t i = 0; i < 1e5; ++i) { samples.push_back(beta_dis(gen)); }
    double acc           = std::accumulate(samples.begin(), samples.end(), 0.0);
    double expected_mean = a / (a + b);
    double mean          = acc / samples.size();
    CHECK(mean == Catch::Approx(expected_mean).epsilon(1e-2));

    double std_dev = std::accumulate(samples.begin(),
                                     samples.end(),
                                     0.0,
                                     [mean](double a, double b) -> double {
                                       return a + (b - mean) * (b - mean);
                                     });

    std_dev /= samples.size();
    double variance = (a * b) / ((a + b) * (a + b) * (a + b + 1));
    CHECK(std_dev == Catch::Approx(variance).epsilon(1e-2));

    std::sort(samples.begin(), samples.end());
    double expected_median = (a - 1.0 / 3.0) / (a + b - 2.0 / 3.0);
    CHECK(samples[static_cast<size_t>(samples.size() / 2)] ==
          Catch::Approx(expected_median).epsilon(1e-2));
  }

  SECTION("mode of 0.5") {
    auto [a, b] = make_ab(0.5, 5);
    beta_distribution<double> beta_dis(a, b);
    std::vector<double>       samples;
    for (size_t i = 0; i < 1e5; ++i) { samples.push_back(beta_dis(gen)); }
    double acc           = std::accumulate(samples.begin(), samples.end(), 0.0);
    double expected_mean = a / (a + b);
    double mean          = acc / samples.size();
    CHECK(mean == Catch::Approx(expected_mean).epsilon(1e-2));

    double std_dev = std::accumulate(samples.begin(),
                                     samples.end(),
                                     0.0,
                                     [mean](double a, double b) -> double {
                                       return a + (b - mean) * (b - mean);
                                     });

    std_dev /= samples.size();
    double variance = (a * b) / ((a + b) * (a + b) * (a + b + 1));
    CHECK(std_dev == Catch::Approx(variance).epsilon(1e-2));

    std::sort(samples.begin(), samples.end());
    double expected_median = (a - 1.0 / 3.0) / (a + b - 2.0 / 3.0);
    CHECK(samples[static_cast<size_t>(samples.size() / 2)] ==
          Catch::Approx(expected_median).epsilon(1e-2));
  }

  SECTION("mode of 0.25") {
    auto [a, b] = make_ab(0.25, 5);
    beta_distribution<double> beta_dis(a, b);
    std::vector<double>       samples;
    for (size_t i = 0; i < 1e6; ++i) { samples.push_back(beta_dis(gen)); }
    double acc           = std::accumulate(samples.begin(), samples.end(), 0.0);
    double expected_mean = a / (a + b);
    double mean          = acc / samples.size();
    CHECK(mean == Catch::Approx(expected_mean).epsilon(1e-2));

    double std_dev = std::accumulate(samples.begin(),
                                     samples.end(),
                                     0.0,
                                     [mean](double a, double b) -> double {
                                       return a + (b - mean) * (b - mean);
                                     });

    std_dev /= samples.size();
    double variance = (a * b) / ((a + b) * (a + b) * (a + b + 1));
    CHECK(std_dev == Catch::Approx(variance).epsilon(1e-2));

    std::sort(samples.begin(), samples.end());
    double expected_median = (a - 1.0 / 3.0) / (a + b - 2.0 / 3.0);
    CHECK(samples[static_cast<size_t>(samples.size() / 2)] ==
          Catch::Approx(expected_median).epsilon(1e-2));
  }

  SECTION("mode of 0.75") {
    auto [a, b] = make_ab(0.75, 5);
    beta_distribution<double> beta_dis(a, b);
    std::vector<double>       samples;
    for (size_t i = 0; i < 1e6; ++i) { samples.push_back(beta_dis(gen)); }
    double acc           = std::accumulate(samples.begin(), samples.end(), 0.0);
    double expected_mean = a / (a + b);
    double mean          = acc / samples.size();
    CHECK(mean == Catch::Approx(expected_mean).epsilon(1e-2));

    double std_dev = std::accumulate(samples.begin(),
                                     samples.end(),
                                     0.0,
                                     [mean](double a, double b) -> double {
                                       return a + (b - mean) * (b - mean);
                                     });

    std_dev /= samples.size();
    double variance = (a * b) / ((a + b) * (a + b) * (a + b + 1));
    CHECK(std_dev == Catch::Approx(variance).epsilon(1e-2));

    std::sort(samples.begin(), samples.end());
    double expected_median = (a - 1.0 / 3.0) / (a + b - 2.0 / 3.0);
    CHECK(samples[static_cast<size_t>(samples.size() / 2)] ==
          Catch::Approx(expected_median).epsilon(1e-2));
  }

  SECTION("mode of 0.75, steeper curve") {
    auto [a, b] = make_ab(0.75, 100);
    beta_distribution<double> beta_dis(a, b);
    std::vector<double>       samples;
    for (size_t i = 0; i < 1e5; ++i) { samples.push_back(beta_dis(gen)); }
    double acc           = std::accumulate(samples.begin(), samples.end(), 0.0);
    double expected_mean = a / (a + b);
    double mean          = acc / samples.size();
    CHECK(mean == Catch::Approx(expected_mean).epsilon(1e-2));

    double std_dev = std::accumulate(samples.begin(),
                                     samples.end(),
                                     0.0,
                                     [mean](double a, double b) -> double {
                                       return a + (b - mean) * (b - mean);
                                     });

    std_dev /= samples.size();
    double variance = (a * b) / ((a + b) * (a + b) * (a + b + 1));
    CHECK(std_dev == Catch::Approx(variance).epsilon(1e-2));

    std::sort(samples.begin(), samples.end());
    double expected_median = (a - 1.0 / 3.0) / (a + b - 2.0 / 3.0);
    CHECK(samples[static_cast<size_t>(samples.size() / 2)] ==
          Catch::Approx(expected_median).epsilon(1e-2));
  }
}
