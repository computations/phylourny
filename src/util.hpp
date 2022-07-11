#ifndef UTIL_HPP
#define UTIL_HPP

#include <functional>
#include <random>
#include <string>
#include <sul/dynamic_bitset.hpp>
#include <vector>

typedef std::vector<std::vector<double>> matrix_t;
typedef std::vector<double>              vector_t;
typedef std::vector<double>              params_t;
typedef sul::dynamic_bitset<>            tip_bitset_t;
typedef std::mt19937_64                  random_engine_t;
typedef size_t                           clock_tick_t;

/**
 * TEST INFO FROM HEADER
 */
matrix_t uniform_matrix_factory(size_t n);
matrix_t random_matrix_factory(size_t n, uint64_t seed);

std::string to_json(const matrix_t &m);
std::string to_json(const vector_t &m);
std::string to_json(const std::vector<size_t> &m);

std::string to_string(const matrix_t &m);
std::string to_string(const vector_t &m);
std::string to_string(const std::vector<size_t> &m);

vector_t softmax(const vector_t &v);

std::string compute_base26(size_t i);

template <typename result_type> class beta_distribution {
public:
  beta_distribution() = delete;
  beta_distribution(result_type alpha,
                    result_type beta,
                    result_type theta = 1.0) :
      g1{alpha, theta}, g2{beta, theta} {}
  template <class generator_type> result_type operator()(generator_type &gen) {
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
constexpr inline std::pair<double, double> make_ab(double median, double k) {
  double a = median * (k - 2) + 1;
  double b = k - a;
  return std::make_pair(a, b);
}

params_t update_win_probs(const params_t &params, random_engine_t &gen);
std::function<params_t(const params_t &, random_engine_t &)>
update_poission_model_factory(double sigma);

double skellam_pmf(int k, double u1, double u2);
double skellam_cmf(int k, double u1, double u2);

#endif // UTIL_HPP
