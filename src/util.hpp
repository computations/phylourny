#ifndef UTIL_HPP
#define UTIL_HPP

#include <functional>
#include <random>
#include <string>
#include <sul/dynamic_bitset.hpp>
#include <vector>

using matrix_t        = std::vector<std::vector<double>>;
using vector_t        = std::vector<double>;
using params_t        = std::vector<double>;
using tip_bitset_t    = sul::dynamic_bitset<>;
using random_engine_t = std::mt19937_64;
using clock_tick_t    = size_t;

/**
 * TEST INFO FROM HEADER
 */
auto uniform_matrix_factory(size_t n) -> matrix_t;
auto random_matrix_factory(size_t n, uint64_t seed) -> matrix_t;

auto to_json(const matrix_t &m) -> std::string;
auto to_json(const vector_t &m) -> std::string;
auto to_json(const std::vector<size_t> &m) -> std::string;

auto to_string(const matrix_t &m) -> std::string;
auto to_string(const vector_t &m) -> std::string;
auto to_string(const std::vector<size_t> &m) -> std::string;

auto softmax(const vector_t &v) -> vector_t;

auto compute_base26(size_t i) -> std::string;

template <typename result_type> class beta_distribution {
public:
  beta_distribution() = delete;
  beta_distribution(result_type alpha,
                    result_type beta,
                    result_type theta = 1.0) :
      g1{alpha, theta}, g2{beta, theta} {}
  template <class generator_type>
  auto operator()(generator_type &gen) -> result_type {
    result_type x = g1(gen);
    result_type y = g2(gen);
    return x / (x + y);
  }

private:
  std::gamma_distribution<result_type> g1;
  std::gamma_distribution<result_type> g2;
};

/**
 * Generate an a and b suitable for configuring a beta distrbution. The median
 * of the distrbution will be `median`, and the concentration will be `k`.
 */
constexpr inline auto make_ab(double median, double k)
    -> std::pair<double, double> {
  double a = median * (k - 2) + 1;
  double b = k - a;
  return std::make_pair(a, b);
}

auto update_win_probs(const params_t &params, random_engine_t &gen) -> params_t;
auto update_poission_model_factory(double sigma)
    -> std::function<params_t(const params_t &, random_engine_t &)>;

auto skellam_pmf(int k, double u1, double u2) -> double;
auto skellam_cmf(int k, double u1, double u2) -> double;

auto gamma_prior(const params_t &) -> double;
auto uniform_prior(const params_t &) -> double;
auto normal_prior(const params_t &) -> double;

#endif // UTIL_HPP
