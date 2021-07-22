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
#include <sul/dynamic_bitset.hpp>
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
                              tournament_edge_t::edge_type_e::win}},
      _single_mode{false} {
    relabel_indicies();
  }

  tournament_t(tournament_node_t &&head) :
      _head{std::move(head)}, _single_mode{false} {}

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
  void relabel_indicies() {
    _head.relabel_indicies(0);
    _head.set_tip_bitset(tip_count());
  }

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
    _head.set_tip_bitset(labels.size());
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
  vector_t eval() {
    if (!check_matrix_size(_win_probs)) {
      throw std::runtime_error("Initialize the win probs before calling eval");
    }
    _head.reset_saved_evals();
    if (_single_mode) {
      vector_t result;
      result.resize(tip_count());

      sul::dynamic_bitset<> include(tip_count());
      include.flip();

      for (size_t i = 0; i < result.size(); i++) {
        result[i] = _head.single_eval(_win_probs, i, include);
      }

      return result;
    }
    return _head.eval(_win_probs, tip_count());
  }

  void set_single_mode() { _single_mode = true; }

  void set_single_mode(bool m) { _single_mode = m; }

private:
  bool check_matrix_size(const matrix_t &wp) const {
    auto tipc = tip_count();
    return tipc == wp.size();
  }

  tournament_node_t _head;
  matrix_t          _win_probs;
  bool              _single_mode;
};

#endif
