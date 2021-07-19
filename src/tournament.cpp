#include "debug.h"
#include "tournament.hpp"
#include <algorithm>
#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <stdint.h>

std::shared_ptr<tournament_node_t>
tournament_node_factory(size_t sub_tourny_size) {
  if (sub_tourny_size == 1) {
    return std::shared_ptr<tournament_node_t>{new tournament_node_t{}};
  }
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
    size_t                                          cur_size = nodes.size() / 2;
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

tournament_t tournament_factory(const std::vector<std::string> &team_labels) {
  auto t = tournament_factory(team_labels.size());
  t.relabel_tips(team_labels);
  return t;
}

bool tournament_node_t::is_tip() const {
  return std::holds_alternative<team_t>(_children);
}

size_t tournament_node_t::tip_count() const {
  if (is_tip()) return 1;
  size_t t_c = 0;
  if (children().left.is_win()) { t_c += children().left->tip_count(); }
  if (children().right.is_win()) { t_c += children().right->tip_count(); }

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
  if (children().left.is_win()) { children().left->relabel_tips(labels); }
  if (children().right.is_win()) { children().right->relabel_tips(labels); }
}

bool tournament_node_t::is_member(size_t index) const {
  if (is_tip()) { return team().index == index; }
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
                                 size_t          tip_count) const {

  if (is_tip()) {
    vector_t wpv(tip_count);
    wpv[team().index] = 1.0;
    return wpv;
  }

  /* This will be correct, but inefficient, as it does not take into account
   * that the match might have already been evaluated. A future optimization
   * would be to include a "evaluated" flag so that the evaluation of the
   * current node can be skipped. Right now, since we are computing the results
   * for a single elimination tournament, we don't need to do this.
   */
  auto l_wpv  = children().left.eval(pmatrix, tip_count);
  auto r_wpv  = children().right.eval(pmatrix, tip_count);
  auto bestof = children().bestof;

  auto fold_a = fold(l_wpv, r_wpv, bestof, pmatrix);
  debug_print(EMIT_LEVEL_DEBUG, "fold_a: %s", to_string(fold_a).c_str());
  auto fold_b = fold(r_wpv, l_wpv, bestof, pmatrix);
  debug_print(EMIT_LEVEL_DEBUG, "fold_b: %s", to_string(fold_b).c_str());
  for (size_t i = 0; i < fold_a.size(); ++i) { fold_a[i] += fold_b[i]; }
  debug_print(EMIT_LEVEL_DEBUG, "eval result: %s", to_string(fold_a).c_str());
  return fold_a;
}

/**
 * A "fold" what I call each term of the main formula for evaluation. In the
 * expression
 *
 * @f[
 * R_i = x_i \times \sum_j \left( y_j \times P_{i \vdash j} \right)
 *     + y_i \times \sum_j \left( x_j \times P_{i \vdash j} \right)
 * @f]
 *
 * Where @f$ x_i @f$ is the @f$ i @f$th term of the WPV @f$ x @f$.Afold then, is
 * the one of the two terms.
 *
 * @param x This corresponds to the `x` in the left fold above.
 *
 * @param y This corresponds to the `y` in the left fold above.
 *
 * @param bestof How many games are to be played.
 *
 * @param pmatrix The pairwise win probability matrix.
 */
vector_t tournament_node_t::fold(const vector_t &x,
                                 const vector_t &y,
                                 uint64_t        bestof,
                                 const matrix_t &pmatrix) const {
  vector_t r(x.size());
  for (size_t m1 = 0; m1 < x.size(); ++m1) {
    if (x[m1] == 0.0) { continue; }
    for (size_t m2 = 0; m2 < y.size(); ++m2) {
      r[m1] += bestof_n(pmatrix[m1][m2], pmatrix[m2][m1], bestof) * y[m2];
    }

    /* This line was added to make it so that multi-elimination tournaments
     * compute a "correct" wpv. This is to say, that the wpv will sum to 1.
     *
     * The problem is, this isn't correct. From a standpoint about the
     * probabiltiy, it doesn't make any sense. So, I need to do something about
     * this.
     */
    r[m1] *= x[m1] / (1.0 - y[m1]);
  }
  return r;
}

vector_t tournament_edge_t::eval(const matrix_t &pmatrix,
                                 size_t          tip_count) const {
  auto r = _node->eval(pmatrix, tip_count);
  return r;
}

bool tournament_node_t::is_simple() const {
  if (is_tip()) { return true; }
  return std::get<match_parameters_t>(_children).is_simple();
}

bool tournament_edge_t::is_simple() const {
  if (is_loss()) { return false; }
  return _node->is_simple();
}
