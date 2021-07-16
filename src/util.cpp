#include "util.hpp"

#include <cmath>
#include <iomanip>
#include <random>

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
  matrix_t matrix;
  std::mt19937_64 rng(seed);
  std::uniform_real_distribution<> dist;
  for (size_t i = 0; i < n; ++i) {
    matrix.emplace_back(n);
  }
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
  for (auto f : v) {
    ent += -f * log2(f);
  }
  return ent;
}

double compute_perplexity(const vector_t &v) {
  double ent = compute_entropy(v);
  return pow(2.0, ent);
}

std::string to_json(const matrix_t &m) {
  std::stringstream out;
  out << std::setprecision(3);
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
  out << std::setprecision(3);
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
  out << std::setprecision(3);
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
  out << std::setprecision(3);
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
