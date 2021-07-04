#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include "debug.h"
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <utility>
#include <variant>
#include <vector>

typedef std::vector<std::vector<double>> matrix_t;
typedef std::vector<double> vector_t;

matrix_t uniform_matrix_factory(size_t n);
matrix_t random_matrix_factory(size_t n, uint64_t seed);

double compute_entropy(const vector_t &v);

double compute_perplexity(const vector_t &v);

std::string to_json(const matrix_t &m);
std::string to_json(const vector_t &m);
std::string to_json(const std::vector<size_t> &m);

std::string to_string(const matrix_t &m);
std::string to_string(const vector_t &m);
std::string to_string(const std::vector<size_t> &m);

constexpr double factorial_table[] = {
    1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800,
};

constexpr size_t factorial_table_size =
    sizeof(factorial_table) / sizeof(double);

constexpr inline double factorial(uint64_t i) {
  if (i < factorial_table_size) {
    return factorial_table[i];
  }
  double f = factorial_table[factorial_table_size - 1];
  for (size_t k = factorial_table_size; k <= i; ++k) {
    f *= k;
  }
  return f;
}

constexpr inline double combinations(uint64_t n, uint64_t i) {
  return factorial(n) / (factorial(i) * factorial(n - i));
}

constexpr inline double int_pow(double base, uint64_t k) {
  if (k == 0) {
    return 1.0;
  }
  double wpp1 = base;
  for (size_t i = 1; i < k; ++i) {
    wpp1 *= base;
  }
  return wpp1;
}

constexpr inline double bestof_n(double wp1, double wp2, uint64_t n) {
  uint64_t k = (n + 1) / 2;
  double sum = 0.0;
  double wpp1 = int_pow(wp1, k);
  for (size_t i = 0; i < k; ++i) {
    sum += int_pow(wp2, i) * combinations(k + i - 1, i);
  }
  return sum * wpp1;
}

struct team_t {
  std::string label;
  size_t index;
};

class tournament_node_t;

class tournament_edge_t {
public:
  enum class edge_type_e {
    win,
    loss,
  };

  /* Constructors */
  tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_e::win} {}
  tournament_edge_t(const std::shared_ptr<tournament_node_t> &node,
                    edge_type_e edge_type)
      : _node{std::move(node)}, _edge_type{edge_type} {}
  tournament_edge_t(tournament_node_t *node, edge_type_e edge_type)
      : _node{node}, _edge_type{edge_type} {}

  /* Operators */
  tournament_node_t &operator*() { return *_node; }
  const tournament_node_t *operator->() const { return _node.get(); }
  tournament_node_t *operator->() { return _node.get(); }
  operator bool() const { return _node != nullptr; }

  /* Attributes */
  bool empty() const { return _node == nullptr; }
  inline bool is_win() const { return _edge_type == edge_type_e::win; }
  inline bool is_loss() const { return !is_win(); }

  inline vector_t eval(const matrix_t &pmatrix, size_t tip_count) const;

private:
  /* Data Members */
  std::shared_ptr<tournament_node_t> _node;
  edge_type_e _edge_type;
};

struct tournament_children_t {
  tournament_edge_t left;
  tournament_edge_t right;
  uint64_t bestof = 1;
};

class tournament_node_t {
public:
  tournament_node_t() : _children{team_t()} {}
  tournament_node_t(team_t t) : _children{t} {}
  tournament_node_t(std::string team_name) : _children{team_t{team_name, 0}} {}

  tournament_node_t(const tournament_children_t &c) : _children{c} {}
  tournament_node_t(const tournament_edge_t &l, const tournament_edge_t &r)
      : tournament_node_t{tournament_children_t{l, r}} {}
  tournament_node_t(const std::shared_ptr<tournament_node_t> &l,
                    tournament_edge_t::edge_type_e lt,
                    const std::shared_ptr<tournament_node_t> &r,
                    tournament_edge_t::edge_type_e rt)
      : tournament_node_t{tournament_edge_t{l, lt}, tournament_edge_t{r, rt}} {}
  tournament_node_t(const std::shared_ptr<tournament_node_t> &l,
                    const std::shared_ptr<tournament_node_t> &r)
      : tournament_node_t{l, tournament_edge_t::edge_type_e::win, r,
                          tournament_edge_t::edge_type_e::win} {}

  bool is_tip() const;
  size_t tip_count() const;
  bool is_member(size_t index) const;

  void label_map(std::vector<std::pair<std::string, size_t>> &lm) const;
  void relabel_tips(const std::vector<std::string> labels);
  size_t relabel_indicies(size_t index);

  inline std::vector<size_t> members(size_t node_count) const;

  vector_t eval(const matrix_t &pmatrix, size_t tip_count) const;
  vector_t fold(const vector_t &wpv1, const vector_t &wpv2, uint64_t bestof,
                const matrix_t &pmatrix) const;

private:
  inline const tournament_children_t &children() const {
    return std::get<tournament_children_t>(_children);
  }
  inline tournament_children_t &children() {
    return std::get<tournament_children_t>(_children);
  }
  inline const team_t &team() const { return std::get<team_t>(_children); }
  inline team_t &team() { return std::get<team_t>(_children); }

  /* Data Members */
  std::variant<tournament_children_t, team_t> _children;
};

class tournament_t {
public:
  tournament_t()
      : _head{tournament_edge_t{new tournament_node_t,
                                tournament_edge_t::edge_type_e::win},
              tournament_edge_t{new tournament_node_t,
                                tournament_edge_t::edge_type_e::win}} {
    relabel_indicies();
  }

  tournament_t(tournament_node_t &&head) : _head{std::move(head)} {}

  tournament_t(const tournament_node_t &) = delete;

  size_t tip_count() const { return _head.tip_count(); }
  void reset_win_probs(const matrix_t wp) {
    if (check_matrix_size(wp)) {
      _win_probs = wp;
    } else {
      throw std::runtime_error("Matrix is the wrong size for the tournament");
    }
  }

  void relabel_indicies() { _head.relabel_indicies(0); }
  void relabel_tips(const std::vector<std::string> &labels) {
    if (tip_count() > labels.size()) {
      throw std::runtime_error("Labels vector is too small");
    }
    _head.relabel_tips(labels);
  }

  std::vector<std::pair<std::string, size_t>> label_map() {
    relabel_indicies();
    std::vector<std::pair<std::string, size_t>> lm;
    _head.label_map(lm);
    return lm;
  }

  vector_t eval() const {
    if (!check_matrix_size(_win_probs)) {
      throw std::runtime_error("Initialize the win probs before calling eval");
    }
    return _head.eval(_win_probs, tip_count());
  }

private:
  bool check_matrix_size(const matrix_t &wp) const {
    auto tipc = tip_count();
    return tipc == wp.size();
  }

  tournament_node_t _head;
  matrix_t _win_probs;
};

tournament_t tournament_factory(size_t tourny_size);
tournament_t tournament_factory(const std::vector<std::string> &);
tournament_t tournament_factory(size_t tourny_size_l, size_t tourny_size_r);

std::shared_ptr<tournament_node_t>
tournament_node_factory(size_t sub_tourny_size);

#endif
