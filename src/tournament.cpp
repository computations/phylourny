#include "debug.h"
#include "simulation_node.hpp"
#include "single_node.hpp"
#include "tournament.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"
#include "util.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdint.h>

template <> void tournament_t<tournament_node_t>::relabel_indicies() {
  _head->assign_internal_labels();
  _head->relabel_indicies(0);
  _head->set_tip_bitset(tip_count());
}

template <> void tournament_t<single_node_t>::relabel_indicies() {
  _head->assign_internal_labels();
  _head->relabel_indicies(0);
  _head->set_tip_bitset(tip_count());
  dynamic_cast<single_node_t &>(*_head).init_assigned_teams();
}

template <> void tournament_t<simulation_node_t>::relabel_indicies() {
  _head->assign_internal_labels();
  _head->relabel_indicies(0);
  _head->set_tip_bitset(tip_count());
}

template <> vector_t tournament_t<simulation_node_t>::eval(size_t iters) {
  if (!check_matrix_size(_win_probs)) {
    throw std::runtime_error("Initialize the win probs before calling eval");
  }
  return _head->eval(_win_probs, tip_count(), iters);
}
