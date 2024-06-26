#include "simulation_node.hpp"
#include "single_node.hpp"
#include "tournament.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"
#include "util.hpp"
#include <cstdlib>

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

template <>
auto tournament_t<simulation_node_t>::eval(size_t iters) -> vector_t {
  if (!check_matrix_size(_win_probs)) {
    throw std::runtime_error("Initialize the win probs before calling eval");
  }
#ifdef PHYLOURNY_EVAL_TIMES
  auto start_time = std::chrono::high_resolution_clock::now();
#endif

  auto ret = _head->eval(_win_probs, tip_count(), iters);

#ifdef PHYLOURNY_EVAL_TIMES
  auto end_time = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::micro> dur = end_time - start_time;
  debug_print(EMIT_LEVEL_INFO,
              "Simulation Eval (iters: %lu) took: %f microseconds",
              iters,
              dur.count());
#endif

  return ret;
}

extern template class tournament_t<tournament_node_t>;
extern template class tournament_t<single_node_t>;
extern template class tournament_t<simulation_node_t>;
