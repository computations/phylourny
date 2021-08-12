#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include "debug.h"
#include "factorial.hpp"
#include "single_node.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"
#include "util.hpp"
#include <cstddef>
#include <exception>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sul/dynamic_bitset.hpp>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

/**
 * Class that contains the entire tournament structure
 */
template <typename T> class tournament_t {
  static_assert(std::is_base_of<tournament_node_t, T>(),
                "tournament_t is restricted to tournamnet_node_t only");

public:
  /**
   * Default constructor currently makes a 2 node tournament. In the future I
   * will delete it.
   */
  tournament_t() :
      _head{
          new T{tournament_edge_t{new T, tournament_edge_t::edge_type_e::win},
                tournament_edge_t{new T, tournament_edge_t::edge_type_e::win}}},
      _single_mode{false} {
    relabel_indicies();
  }

  tournament_t(std::unique_ptr<T> &&head) :
      _head{std::move(head)}, _single_mode{false} {
    relabel_indicies();
  }

  tournament_t(T *head) : tournament_t{std::unique_ptr<T>{head}} {}

  tournament_t(const T &) = delete;

  size_t tip_count() const { return _head->tip_count(); }
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
  void relabel_indicies();

  /**
   * Relabel the tips based on an index to label map.
   *
   * @param labels A map from team indices to labels.
   */
  void relabel_tips(const std::vector<std::string> &labels) {
    if (tip_count() > labels.size()) {
      throw std::runtime_error("Labels vector is too small");
    }
    _head->relabel_tips(labels);
    _head->set_tip_bitset(labels.size());
  }

  /**
   * Makes a map from labels to indices for the tournament.
   *
   * @return A map from labels to indices.
   */
  std::vector<std::pair<std::string, size_t>> label_map() {
    relabel_indicies();
    std::vector<std::pair<std::string, size_t>> lm;
    _head->label_map(lm);
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
    _head->reset_saved_evals();
    return _head->eval(_win_probs, tip_count());
  }

  vector_t eval(size_t iters);

  void set_single_mode() { _single_mode = true; }

  void set_single_mode(bool m) { _single_mode = m; }

  std::string dump_state_graphviz() const {
    std::ostringstream oss;
    dump_state_graphviz(oss);
    return oss.str();
  }

  void dump_state_graphviz_scratchpad(std::ostream &os) const {
    auto node_attr_func = [](const tournament_node_t &n) -> std::string {
      std::ostringstream oss;
      oss << "[label=";
      if (n.is_tip()) {
        oss << "\"" << n.get_display_label() << "|" << n.get_team_index()
            << "\" ";
      } else {
        auto sp = n.get_scratch_pad();
        if (!n.is_simple()) {
          oss << "\"" << sp.fold_l << "|" << sp.fold_r << "|" << sp.result
              << "|" << sp.eval_index << "|" << sp.include.to_string() << "\" ";
        } else {
          oss << sp.result << " ";
          oss << "style = filled ";
        }
      }
      oss.seekp(-1, std::ios_base::end);
      oss << "]";
      return oss.str();
    };

    auto edge_attr_func = [](const tournament_edge_t &e) -> std::string {
      std::ostringstream oss;
      oss << "[";

      if (e.is_win()) {
        oss << "style = solid ";
      } else {
        oss << "style = dashed ";
      }

      oss.seekp(-1, std::ios_base::end);
      oss << "]";
      return oss.str();
    };

    dump_state_graphviz(os, node_attr_func, edge_attr_func);
  }

  void dump_state_graphviz(std::ostream &os) const {
    auto node_attr_func = [](const tournament_node_t &n) -> std::string {
      std::ostringstream oss;
      oss << "[label=";
      if (n.is_tip()) {
        oss << n.get_display_label() << " ";
      } else {
        auto mv = n.get_memoized_values();
        oss << "\"";
        for (size_t i = 0; i < mv.size(); i++) {
          oss << mv[i];
          if (i != mv.size() - 1) { oss << "|"; }
        }
        oss << "\" ";
      }
      oss.seekp(-1, std::ios_base::end);
      oss << "]";
      return oss.str();
    };

    auto edge_attr_func = [](const tournament_edge_t &e) -> std::string {
      std::ostringstream oss;
      oss << "[";

      if (e.is_win()) {
        oss << "style = solid ";
      } else {
        oss << "style = dashed ";
      }

      oss.seekp(-1, std::ios_base::end);
      oss << "]";
      return oss.str();
    };

    dump_state_graphviz(os, node_attr_func, edge_attr_func);
  }

  void dump_state_graphviz(
      std::ostream &os,
      const std::function<std::string(const tournament_node_t &)>
          &node_attr_func,
      const std::function<std::string(const tournament_edge_t &)>
          &edge_attr_func) const {

    os << "digraph {\n";
    os << "node [shape=record]\n";
    _head->dump_state_graphviz(os, node_attr_func, edge_attr_func);
    os << "}";
  }

private:
  bool check_matrix_size(const matrix_t &wp) const {
    auto tipc = tip_count();
    return tipc == wp.size();
  }

  std::unique_ptr<T> _head;
  matrix_t           _win_probs;
  bool               _single_mode;
};

#endif
