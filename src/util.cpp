#include "debug.h"
#include "factorial.hpp"
#include "util.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <random>

constexpr size_t JSON_PRECISION = 14;

/**
 * TEST INFO FROM CPP
 */
matrix_t uniform_matrix_factory(size_t n) {
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

matrix_t random_matrix_factory(size_t n, uint64_t seed) {
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

double compute_entropy(const vector_t &v) {
  double ent = 0.0;
  for (auto f : v) { ent += -f * log2(f); }
  return ent;
}

double compute_perplexity(const vector_t &v) {
  double ent = compute_entropy(v);
  return pow(2.0, ent);
}

std::string to_json(const matrix_t &m) {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  for (auto &i : m) {
    out << "[";
    for (auto &j : i) {
      out << std::setw(3);
      out << std::fixed << j << ", ";
    }
    out.seekp(-1, out.cur);
    out << "],\n";
  }
  out.seekp(-2, out.cur);
  out << "\n";
  auto ret_str = out.str();
  ret_str.resize(ret_str.size() - 1);
  return ret_str;
}

std::string to_json(const vector_t &m) {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  out << "[";
  for (auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << ", ";
  }
  out.seekp(-2, out.cur);
  out << "]";
  auto tmp = out.str();
  tmp.resize(tmp.size() - 1);
  return tmp;
}

std::string to_json(const std::vector<size_t> &m) {
  std::stringstream out;
  out << "[";
  for (auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << ", ";
  }
  out.seekp(-2, out.cur);
  out << "]";
  auto tmp = out.str();
  tmp.resize(tmp.size() - 1);
  return tmp;
}

std::string to_string(const matrix_t &m) {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  for (auto &i : m) {
    out << "[";
    for (auto &j : i) {
      out << std::setw(3);
      out << std::fixed << j << " ";
    }
    out.seekp(-1, out.cur);
    out << "]\n";
  }
  out.seekp(-1, out.cur);
  auto ret_str = out.str();
  ret_str.resize(ret_str.size() - 1);
  return ret_str;
}

std::string to_string(const vector_t &m) {
  std::stringstream out;
  out << std::setprecision(JSON_PRECISION);
  out << "[";
  for (auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << " ";
  }
  out.seekp(-1, out.cur);
  out << "]";
  return out.str();
}

std::string to_string(const std::vector<size_t> &m) {
  std::stringstream out;
  out << "[";
  for (auto &i : m) {
    out << std::setw(3);
    out << std::fixed << i << " ";
  }
  out.seekp(-1, out.cur);
  out << "]";
  return out.str();
}

std::string compute_base26(size_t i) {
  size_t length =
      std::max(static_cast<size_t>(std::ceil(
                   std::log(static_cast<double>(i + 1)) / std::log(26.0))),
               1ul);
  std::string ret;
  ret.reserve(length);

  for (size_t j = 0; j < length; j++) {
    ret += 'a' + (i % 26);
    i /= 26;
  }

  return ret;
}

params_t update_win_probs(const params_t &params, random_engine_t &gen) {
  params_t temp_params{params};
  for (size_t j = 0; j < params.size(); ++j) {
    auto [a, b] = make_ab(params[j], 5);
    beta_distribution<double> bd(a, b);
    temp_params[j] = bd(gen);
  }
  return temp_params;
}

double skellam_pmf(int k, double u1, double u2) {
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

double skellam_cmf(int k, double u1, double u2) {
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

std::function<params_t(const params_t &, random_engine_t &gen)>
update_poission_model_factory(double sigma) {
  auto l = [sigma](const params_t &p, random_engine_t &gen) -> params_t {
    std::normal_distribution<double> dis(0.0, sigma);
    params_t                         tmp(p);
    std::transform(tmp.begin(),
                   tmp.end(),
                   tmp.begin(),
                   [&](double f) -> double { return f + dis(gen); });
    for (auto &f : tmp) { f += dis(gen); }
    return tmp;
  };
  return l;
}
