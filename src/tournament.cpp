#include "debug.h"
#include "tournament.hpp"
#include <algorithm>

std::shared_ptr<tournament_node_t>
tournament_node_factory(size_t sub_tourny_size) {
  if (sub_tourny_size % 2 != 0) {
    throw std::runtime_error(
        "Tournament node factory only accepts powers of 2");
  }

  std::vector<std::unique_ptr<tournament_node_t>> nodes;
  nodes.reserve(sub_tourny_size);
  for (size_t i = 0; i < sub_tourny_size; ++i) {
    nodes.emplace_back(new tournament_node_t{});
  }

  while (nodes.size() != 1) {
    size_t cur_size = nodes.size() / 2;
    std::vector<std::unique_ptr<tournament_node_t>> tmp_nodes;
    tmp_nodes.reserve(cur_size);
    for (size_t i = 0, j = 1; j < nodes.size(); i += 2, j += 2) {
      tmp_nodes.emplace_back(
          new tournament_node_t{std::move(nodes[i]), std::move(nodes[j])});
    }
    std::swap(tmp_nodes, nodes);
  }

  return std::move(nodes[0]);
}

tournament_t tournament_factory(size_t tourny_size) {
  if (tourny_size % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }

  tournament_t t{tournament_node_t{tournament_node_factory(tourny_size / 2),
                                   tournament_node_factory(tourny_size / 2)}};
  t.relabel_indicies();
  return t;
}

tournament_t tournament_factory(size_t tourny_size_l, size_t tourny_size_r) {
  if (tourny_size_r % 2 != 0 || tourny_size_l % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }

  tournament_t t{tournament_node_t{tournament_node_factory(tourny_size_l),
                                   tournament_node_factory(tourny_size_r)}};
  t.relabel_indicies();
  return t;
}

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

bool tournament_node_t::is_tip() const {
  return std::holds_alternative<team_t>(_children);
}

size_t tournament_node_t::tip_count() const {
  if (is_tip())
    return 1;
  size_t t_c = 0;
  if (children().left.is_win()) {
    t_c += children().left->tip_count();
  }
  if (children().right.is_win()) {
    t_c += children().right->tip_count();
  }

  return t_c;
}

size_t tournament_node_t::relabel_indicies(size_t index) {
  if (is_tip()) {
    team().index = index;
    return index + 1;
  }
  if (children().left.is_win()) {
    index = children().left->relabel_indicies(index);
  }
  if (children().right.is_win()) {
    index = children().right->relabel_indicies(index);
  }
  return index;
}

void tournament_node_t::label_map(
    std::vector<std::pair<std::string, size_t>> &lm) const {
  if (is_tip()) {
    lm.emplace_back(team().label, team().index);
    return;
  }
  children().left->label_map(lm);
  children().right->label_map(lm);
}

void tournament_node_t::relabel_tips(const std::vector<std::string> labels) {
  if (is_tip()) {
    team().label = labels[team().index];
    return;
  }
  if (children().left.is_win()) {
    children().left->relabel_tips(labels);
  }
  if (children().right.is_win()) {
    children().right->relabel_tips(labels);
  }
}

bool tournament_node_t::is_member(size_t index) const {
  if (is_tip()) {
    return team().index == index;
  }
  return children().left->is_member(index) ||
         children().right->is_member(index);
}

std::vector<size_t> tournament_node_t::members(size_t node_count) const {
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

vector_t tournament_node_t::eval(const matrix_t &pmatrix,
                                 size_t tip_count) const {

  if (is_tip()) {
    vector_t wpv(tip_count);
    wpv[team().index] = 1.0;
    return wpv;
  }

  auto l_wpv = children().left.eval(pmatrix, tip_count);
  auto r_wpv = children().right.eval(pmatrix, tip_count);

  auto fold_a = fold(l_wpv, r_wpv, pmatrix);
  debug_print(EMIT_LEVEL_IMPORTANT, "fold_a: %s", to_string(fold_a).c_str());
  auto fold_b = fold(r_wpv, l_wpv, pmatrix);
  debug_print(EMIT_LEVEL_IMPORTANT, "fold_b: %s", to_string(fold_b).c_str());
  for (size_t i = 0; i < fold_a.size(); ++i) {
    fold_a[i] += fold_b[i];
  }
  debug_print(EMIT_LEVEL_IMPORTANT, "eval result: %s", to_string(fold_a).c_str());
  return fold_a;
}

vector_t tournament_node_t::fold(const vector_t &w, const vector_t &y,
                                 const matrix_t &p) const {
  vector_t r(w.size());
  for (size_t m1 = 0; m1 < w.size(); ++m1) {
    if(w[m1] == 0.0){
      continue;
    }
    for (size_t m2 = 0; m2 < y.size(); ++m2) {
      r[m1] += p[m1][m2] * y[m2];
    }
    r[m1] *= w[m1] / (1.0 - y[m1]);
  }
  return r;
}

vector_t tournament_edge_t::eval(const matrix_t &pmatrix,
                                 size_t tip_count) const {

  auto r = _node->eval(pmatrix, tip_count);
  return r;
}
