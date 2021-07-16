#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include "debug.h"
#include "factorial.hpp"
#include "util.hpp"
#include <cstddef>
#include <exception>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

struct team_t {
  std::string label;
  size_t      index;
};

class tournament_node_t;

/**
 * A class representing the edge of a tournament. It has 2 "colors", win or
 * lose, which indicates what kind of edge it is. If it is a win edge, then
 * winner of the node travels along this edge to the adjacent node.
 *
 * This edge is directed. This is to say, it isn't a pair of nodes, but only
 * contains the end node. Additionally, the tournament arcs are reversed. This
 * is to say, normally the teams start at the tips and advance in. For the
 * purposes of calculating the result, we reverse all the arcs. So, what is
 * pointed at by this edge is the "child" of some other node.
 *
 * In summary, this is a shared pointer with a color, indicating what team to
 * get from the child.
 */
class tournament_edge_t {
public:
  enum struct edge_type_e {
    win,
    loss,
  };

  tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_e::win} {}
  tournament_edge_t(const std::shared_ptr<tournament_node_t> &node,
                    edge_type_e                               edge_type) :
      _node{std::move(node)}, _edge_type{edge_type} {}
  tournament_edge_t(tournament_node_t *node, edge_type_e edge_type) :
      _node{node}, _edge_type{edge_type} {}

  tournament_node_t &      operator*() { return *_node; }
  const tournament_node_t *operator->() const { return _node.get(); }
  tournament_node_t *      operator->() { return _node.get(); }
                           operator bool() const { return _node != nullptr; }

  bool        empty() const { return _node == nullptr; }
  inline bool is_win() const { return _edge_type == edge_type_e::win; }
  inline bool is_loss() const { return !is_win(); }

  inline vector_t eval(const matrix_t &pmatrix, size_t tip_count) const;

private:
  /* Data Members */
  std::shared_ptr<tournament_node_t> _node;
  edge_type_e                        _edge_type;
};

struct match_parameters_t {
  tournament_edge_t left;
  tournament_edge_t right;
  uint64_t          bestof = 1;
};

class tournament_node_t {
public:
  tournament_node_t() : _children{team_t()} {}
  tournament_node_t(team_t t) : _children{t} {}
  tournament_node_t(std::string team_name) : _children{team_t{team_name, 0}} {}

  tournament_node_t(const match_parameters_t &c) : _children{c} {}
  tournament_node_t(const tournament_edge_t &l, const tournament_edge_t &r) :
      tournament_node_t{match_parameters_t{l, r}} {}
  tournament_node_t(const std::shared_ptr<tournament_node_t> &l,
                    tournament_edge_t::edge_type_e            lt,
                    const std::shared_ptr<tournament_node_t> &r,
                    tournament_edge_t::edge_type_e            rt) :
      tournament_node_t{tournament_edge_t{l, lt}, tournament_edge_t{r, rt}} {}
  tournament_node_t(const std::shared_ptr<tournament_node_t> &l,
                    const std::shared_ptr<tournament_node_t> &r) :
      tournament_node_t{l,
                        tournament_edge_t::edge_type_e::win,
                        r,
                        tournament_edge_t::edge_type_e::win} {}

  bool   is_tip() const;
  size_t tip_count() const;
  bool   is_member(size_t index) const;

  /**
   * Create a map of the team labels to team indexes.
   *
   * @param[out] lm The function will just place the label index pairs at the
   * back of this vector, the vector should be empty when this is called.
   */
  void label_map(std::vector<std::pair<std::string, size_t>> &lm) const;

  /**
   * Label the tips using the vector as a map from team index to team label.
   *
   * @param labels Map of team indices to team labels.
   */
  void relabel_tips(const std::vector<std::string> labels);

  /**
   * Relabel the team indices starting at `index`. Traverses the tree in a
   * preorder fashion, descending the left child first.
   */
  size_t relabel_indicies(size_t index);

  /**
   * Get a list of all the children of this node, and return it as a vector.
   */
  inline std::vector<size_t> members(size_t node_count) const;

  vector_t eval(const matrix_t &pmatrix, size_t tip_count) const;
  vector_t fold(const vector_t &x,
                const vector_t &y,
                uint64_t        bestof,
                const matrix_t &pmatrix) const;

private:
  inline const match_parameters_t &children() const {
    return std::get<match_parameters_t>(_children);
  }
  inline match_parameters_t &children() {
    return std::get<match_parameters_t>(_children);
  }
  inline const team_t &team() const { return std::get<team_t>(_children); }
  inline team_t &      team() { return std::get<team_t>(_children); }

  /* Data Members */
  std::variant<match_parameters_t, team_t> _children;
};

/**
 * Class that contains the entire tournament structure
 */
class tournament_t {
public:
  /**
   * Default constructor currently makes a 2 node tournament. In the future I
   * will delete it.
   */
  tournament_t() :
      _head{tournament_edge_t{new tournament_node_t,
                              tournament_edge_t::edge_type_e::win},
            tournament_edge_t{new tournament_node_t,
                              tournament_edge_t::edge_type_e::win}} {
    relabel_indicies();
  }

  tournament_t(tournament_node_t &&head) : _head{std::move(head)} {}

  tournament_t(const tournament_node_t &) = delete;

  size_t tip_count() const { return _head.tip_count(); }
  void   reset_win_probs(const matrix_t wp) {
    if (check_matrix_size(wp)) {
      _win_probs = wp;
    } else {
      throw std::runtime_error("Matrix is the wrong size for the tournament");
    }
  }

  /**
   * Relabel the indices of the tree. Should be called after adding or removing
   * tips.
   */
  void relabel_indicies() { _head.relabel_indicies(0); }

  /**
   * Relabel the tips based on an index to label map.
   *
   * @param labels A map from team indices to labels.
   */
  void relabel_tips(const std::vector<std::string> &labels) {
    if (tip_count() > labels.size()) {
      throw std::runtime_error("Labels vector is too small");
    }
    _head.relabel_tips(labels);
  }

  /**
   * Makes a map from labels to indices for the tournament.
   *
   * @return A map from labels to indices.
   */
  std::vector<std::pair<std::string, size_t>> label_map() {
    relabel_indicies();
    std::vector<std::pair<std::string, size_t>> lm;
    _head.label_map(lm);
    return lm;
  }

  /**
   * Compute the WPV for the tournament. Please note that nothing is saved
   * between calls to this function. So, while computation is memoized during
   * evaluation, that information is discarded once computation is complete.
   */
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
  matrix_t          _win_probs;
};

tournament_t tournament_factory(size_t tourny_size);
tournament_t tournament_factory(const std::vector<std::string> &);
tournament_t tournament_factory(size_t tourny_size_l, size_t tourny_size_r);

std::shared_ptr<tournament_node_t>
tournament_node_factory(size_t sub_tourny_size);

#endif
