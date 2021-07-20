#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include "debug.h"
#include "factorial.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"
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

#endif
