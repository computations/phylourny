#include "debug.h"
#include "factorial.hpp"
#include "util.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <random>
#include <sstream>

constexpr size_t JSON_PRECISION = 14;

/**
 * TEST INFO FROM CPP
 */
auto uniform_matrix_factory(size_t n) -> matrix_t {
  matrix_t matrix;
  for (size_t i = 0; i < n; ++i) {
    matrix.emplace_back(n);
    for (size_t j = 0; j < n; ++j) {
      if (i == j) {
        matrix[i][j] = 0.0;
      } else {
        matrix[i][j] = 0.5;
      }
    }
  }
  return matrix;
}

auto random_matrix_factory(size_t n, uint64_t seed) -> matrix_t {
  matrix_t                         matrix;
  std::mt19937_64                  rng(seed);
  std::uniform_real_distribution<> dist;
  for (size_t i = 0; i < n; ++i) { matrix.emplace_back(n); }
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = i; j < n; ++j) {
      if (i == j) {
        matrix[i][j] = 0.0;
      } else {
        matrix[i][j] = dist(rng);
        matrix[j][i] = 1 - matrix[i][j];
      }
    }
  }
  return matrix;
}

auto to_json(const matrix_t &m) -> std::string {
  std::ostringstream out;
  out << std::setprecision(JSON_PRECISION);
  for (const auto &i : m) {
    out << "[";
    for (const auto &j : i) {
      out << std::setw(3);
      out << std::fixed << j << ", ";
    }
    out.seekp(-1, std::ostringstream::cur);
    out << "],\n";
  }
  out.seekp(-2, std::ostringstream::cur);
  out << "\n";
  auto ret_str = out.str();
  ret_str.resize(ret_str.size() - 1);
  return ret_str;
}

auto to_json(const vector_t &m) -> std::string {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  out << "[";
  for (const auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << ", ";
  }
  out.seekp(-2, std::stringstream::cur);
  out << "]";
  auto tmp = out.str();
  tmp.resize(tmp.size() - 1);
  return tmp;
}

auto to_json(const std::vector<size_t> &m) -> std::string {
  std::stringstream out;
  out << "[";
  for (const auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << ", ";
  }
  out.seekp(-2, std::stringstream::cur);
  out << "]";
  auto tmp = out.str();
  tmp.resize(tmp.size() - 1);
  return tmp;
}

auto to_string(const matrix_t &m) -> std::string {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  for (const auto &i : m) {
    out << "[";
    for (const auto &j : i) {
      out << std::setw(3);
      out << std::fixed << j << " ";
    }
    out.seekp(-1, std::stringstream::cur);
    out << "]\n";
  }
  out.seekp(-1, std::stringstream::cur);
  auto ret_str = out.str();
  ret_str.resize(ret_str.size() - 1);
  return ret_str;
}

auto to_string(const vector_t &m) -> std::string {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  out << "[";
  for (const auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << " ";
  }
  out.seekp(-1, std::stringstream::cur);
  out << "]";
  return out.str();
}

auto to_string(const std::vector<size_t> &m) -> std::string {
  std::stringstream out;
  out << "[";
  for (const auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << " ";
  }
  out.seekp(-1, std::stringstream::cur);
  out << "]";
  return out.str();
}

auto compute_base26(size_t i) -> std::string {
  size_t length =
      std::max(static_cast<size_t>(std::ceil(
                   std::log(static_cast<double>(i + 1)) / std::log(26.0))),
               1UL);
  std::string ret;
  ret.reserve(length);

  for (size_t j = 0; j < length; j++) {
    ret += 'a' + (i % 26);
    i /= 26;
  }

  return ret;
}

auto update_win_probs(const params_t &params, random_engine_t &gen) -> params_t {
  params_t temp_params{params};
  for (size_t j = 0; j < params.size(); ++j) {
    auto [a, b] = make_ab(params[j], 5);
    beta_distribution<double> bd(a, b);
    temp_params[j] = bd(gen);
  }
  return temp_params;
}

auto skellam_pmf(int k, double u1, double u2) -> double {
  double           p       = 0.0;
  constexpr double epsilon = std::numeric_limits<double>::epsilon();

  for (int i = std::max(0, -k);; ++i) {
    double numerator   = std::pow(u1, k + i) * std::pow(u2, i);
    double denominator = factorial(static_cast<uint64_t>(i)) *
                         factorial(static_cast<uint64_t>(k + i));

    double total = numerator / denominator;
    p += total;
    if (total < epsilon) { break; }
  }
  double factor = std::exp(-(u1 + u2));
  return p * factor;
}

auto skellam_cmf(int k, double u1, double u2) -> double {
  constexpr double epsilon = std::numeric_limits<double>::epsilon();
  double           p       = 0.0;

  double last = 0.0;
  for (int i = k;; --i) {
    double total = skellam_pmf(i, u1, u2);
    p += total;
    if (total < epsilon && total < last) { break; }
    last = total;
  }
  return p;
}

auto
update_poission_model_factory(double sigma) -> std::function<params_t(const params_t &, random_engine_t &gen)> {
  auto l = [sigma](const params_t &p, random_engine_t &gen) -> params_t {
    std::normal_distribution<double>      dis(0.0, sigma);
    std::uniform_int_distribution<size_t> picker(0, p.size() - 1);
    params_t                              tmp(p);
    /*
    std::transform(tmp.begin(),
                   tmp.end(),
                   tmp.begin(),
                   [&](double f) -> double { return f + dis(gen); });
   */
    tmp[picker(gen)] += dis(gen);
    return tmp;
  };
  return l;
}

auto gamma_prior(const params_t &params) -> double {
  constexpr double alpha = 1.0;
  constexpr double beta  = 1.0;
  double           prob  = 1.0;
  for (double par : params) {
    prob *= std::pow(beta, alpha) * std::pow(par, alpha - 1) *
            std::exp(-beta * par) / std::tgamma(alpha);
  }
  return prob;
}

auto uniform_prior(const params_t & /*params*/) -> double { return 1.0; }

auto normal_prior(const params_t &params) -> double {

  constexpr double mu     = 0.0;
  constexpr double sigma  = 1.0;
  const double     denom1 = std::sqrt(2 * 3.14159265359);

  double p = 1.0;
  for (auto param : params) {

    double exponent = ((param - mu) / sigma);
    exponent *= exponent;
    exponent *= -0.5;
    p *= 1 / (sigma * denom1) * std::exp(exponent);
  }

  return p;
}
