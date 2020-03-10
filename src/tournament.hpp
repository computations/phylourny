#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include "debug.h"
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

typedef std::vector<std::vector<double>> matrix_t;
typedef std::vector<double> vector_t;

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

inline std::string to_string(const matrix_t &m) {
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

inline std::string to_string(const vector_t &m) {
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

class tournament_node_t;

class tournament_edge_t {
public:
  enum edge_type_t {
    win,
    loss,
  };

  /* Constructors */
  tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_t::win} {}
  tournament_edge_t(std::unique_ptr<tournament_node_t> &&node,
                    edge_type_t edge_type)
      : _node{std::move(node)}, _edge_type{edge_type} {}
  tournament_edge_t(tournament_node_t *node, edge_type_t edge_type)
      : _node{node}, _edge_type{edge_type} {}

  /* Operators */
  tournament_node_t &operator*() { return *_node; }
  const tournament_node_t *operator->() const { return _node.get(); }
  tournament_node_t *operator->() { return _node.get(); }
  operator bool() const { return _node != nullptr; }

  /* Attributes */
  bool empty() const { return _node == nullptr; }

private:
  /* Data Members */
  std::unique_ptr<tournament_node_t> _node;
  edge_type_t _edge_type;
};

struct tournament_children_t {
  tournament_edge_t left;
  tournament_edge_t right;
};

struct team_t {
  std::string label;
  size_t index;
};

class tournament_node_t {
public:
  tournament_node_t() : _children{team_t()} {}
  tournament_node_t(team_t t) : _children{t} {}
  tournament_node_t(std::string team_name) : _children{team_t{team_name, 0}} {}
  tournament_node_t(tournament_children_t &&c) : _children{std::move(c)} {}

  tournament_node_t(tournament_edge_t &&e1, tournament_edge_t &&e2)
      : tournament_node_t{tournament_children_t{std::move(e1), std::move(e2)}} {
  }

  tournament_node_t(std::unique_ptr<tournament_node_t> &&leftc,
                    tournament_edge_t::edge_type_t leftt,
                    std::unique_ptr<tournament_node_t> &&rightc,
                    tournament_edge_t::edge_type_t rightt)
      : tournament_node_t{tournament_edge_t{std::move(leftc), leftt},
                          tournament_edge_t{std::move(rightc), rightt}} {}

  tournament_node_t(std::unique_ptr<tournament_node_t> &&left,
                    std::unique_ptr<tournament_node_t> &&right)
      : tournament_node_t{std::move(left), tournament_edge_t::win,
                          std::move(right), tournament_edge_t::win} {}

  tournament_node_t(const tournament_children_t &) = delete;
  tournament_node_t(const tournament_edge_t &,
                    const tournament_edge_t &) = delete;
  tournament_node_t(const std::unique_ptr<tournament_node_t> &,
                    tournament_edge_t::edge_type_t,
                    const std::unique_ptr<tournament_node_t> &,
                    tournament_edge_t::edge_type_t) = delete;
  tournament_node_t(const std::unique_ptr<tournament_node_t> &,
                    const std::unique_ptr<tournament_node_t> &) = delete;

  bool is_tip() const { return std::holds_alternative<team_t>(_children); }

  size_t tip_count() const {
    if (is_tip())
      return 1;
    auto &children = std::get<tournament_children_t>(_children);
    return children.left->tip_count() + children.right->tip_count();
  }

  size_t relabel_indicies(size_t index) {
    if (is_tip()) {
      team().index = index;
      return index + 1;
    }
    index = children().left->relabel_indicies(index);
    index = children().right->relabel_indicies(index);
    return index;
  }

  void label_map(std::vector<std::pair<std::string, size_t>> &lm) const {
    if (is_tip()) {
      lm.emplace_back(team().label, team().index);
      return;
    }
    children().left->label_map(lm);
    children().right->label_map(lm);
  }

  void relabel_tips(const std::vector<std::string> labels) {
    if (is_tip()) {
      team().label = labels[team().index];
      return;
    }
    children().left->relabel_tips(labels);
    children().right->relabel_tips(labels);
  }

  bool is_member(size_t index) const {
    if (is_tip()) {
      return team().index == index;
    }
    return children().left->is_member(index) ||
           children().right->is_member(index);
  }

  inline std::vector<bool> get_members(size_t node_count) const {
    std::vector<bool> members(node_count);
    for (size_t i = 0; i < node_count; ++i) {
      members[i] = is_member(i);
    }
    return members;
  }

  inline std::vector<size_t> members(size_t node_count) const {
    std::vector<size_t> members;
    members.reserve(node_count);
    size_t total_members = 0;
    for (size_t i = 0; i < node_count; ++i) {
      if (is_member(i)) {
        members.push_back(i);
        total_members++;
      }
    }
    members.resize(total_members);
    return members;
  }

  vector_t eval(const matrix_t &pmatrix, size_t tip_count) const {

    if (is_tip()) {
      vector_t wpv(tip_count);
      debug_print(EMIT_LEVEL_DEBUG, "team index: %lu", team().index);
      wpv[team().index] = 1.0;
      return wpv;
    }

    debug_string(EMIT_LEVEL_DEBUG, "Evaluating children");
    auto left_wpv = children().left->eval(pmatrix, tip_count);
    auto right_wpv = children().right->eval(pmatrix, tip_count);
    debug_string(EMIT_LEVEL_DEBUG, "Evaluating current node");

    debug_print(EMIT_LEVEL_DEBUG, "left child wpv: %s",
                to_string(left_wpv).c_str());
    debug_print(EMIT_LEVEL_DEBUG, "right child wpv: %s",
                to_string(right_wpv).c_str());

    auto fold_a = fold(left_wpv, right_wpv, pmatrix);
    auto fold_b = fold(right_wpv, left_wpv, pmatrix);
    for (size_t i = 0; i < fold_a.size(); ++i) {
      fold_a[i] += fold_b[i];
    }
    return fold_a;
  }

  inline vector_t fold(vector_t wpv1, vector_t wpv2, matrix_t pmatrix) const {
    vector_t cur_wpv(wpv1.size());
    auto local_members = members(pmatrix.size());

    for (auto m1 : local_members) {
      for (auto m2 : local_members) {
        cur_wpv[m1] += wpv1[m1] * wpv2[m2] * pmatrix[m1][m2];
      }
    }

    return cur_wpv;
  }

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
      : _head{
            tournament_edge_t{new tournament_node_t, tournament_edge_t::win},
            tournament_edge_t{new tournament_node_t, tournament_edge_t::win}} {
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

tournament_t tournament_factory(size_t tourny_size) {
  if (tourny_size % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }
  std::vector<std::unique_ptr<tournament_node_t>> nodes;
  nodes.reserve(tourny_size);
  for (size_t i = 0; i < tourny_size; ++i) {
    nodes.emplace_back(new tournament_node_t{});
  }

  while (nodes.size() != 2) {
    size_t cur_size = nodes.size() / 2;
    std::vector<std::unique_ptr<tournament_node_t>> tmp_nodes;
    tmp_nodes.reserve(cur_size);
    for (size_t i = 0, j = 1; j < nodes.size(); i += 2, j += 2) {
      tmp_nodes.emplace_back(
          new tournament_node_t{std::move(nodes[i]), std::move(nodes[j])});
    }
    std::swap(tmp_nodes, nodes);
  }

  tournament_t t{tournament_node_t{std::move(nodes[0]), std::move(nodes[1])}};
  t.relabel_indicies();
  return t;
}

#endif
