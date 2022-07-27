#include "single_node.hpp"
#include "util.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

auto single_node_t::eval(const matrix_t &pmatrix, size_t tip_count)
    -> vector_t {
  init_assigned_teams();
  vector_t results(tip_count);
  for (tick_result_t tr = tick_result_t::success; tr != tick_result_t::finished;
       tr               = tick()) {
    if (valid()) { results[winner()] += single_eval(pmatrix, true); }
  }
  return results;
}

auto single_node_t::eval_debug(const matrix_t    &pmatrix,
                               size_t             tip_count,
                               const std::string &filename_prefix) -> vector_t {
  vector_t results(tip_count);
  size_t   filename_counter = 0;
  for (tick_result_t tr = tick_result_t::success; tr != tick_result_t::finished;
       tr               = tick()) {
    if (valid()) {
      results[winner()] += single_eval(pmatrix, true);

      std::string filename =
          filename_prefix + std::to_string(filename_counter) + ".dot";

      std::ofstream ofs(filename);

      debug_graphviz(ofs);
      filename_counter++;
    }
  }
  return results;
}

auto single_node_t::single_eval(const matrix_t &pmatrix, bool is_winner)
    -> double {
  if (is_tip()) { return is_winner ? 1.0 : 0.0; }
  if (!is_winner) {
    _saved_val = 1.0;
    return _saved_val;
  }

  _saved_val = left_child().single_eval(pmatrix, children().left.is_win()) *
               right_child().single_eval(pmatrix, children().right.is_win()) *
               pmatrix[winner()][loser()];
  return _saved_val;
}

auto single_node_t::is_cherry() const -> bool {
  if (is_tip()) { return false; }
  return left_child().is_tip() && right_child().is_tip();
}

void single_node_t::init_assigned_teams() {
  if (is_tip()) {
    _assigned_team = team().index;
    return;
  }
  right_child().init_assigned_teams();
  left_child().init_assigned_teams();
  assign_team_reset();
}

void single_node_t::assign_team_reset() {
  _assigned_team = get_tip_bitset().find_first();
}

auto single_node_t::tick() -> tick_result_t {
  if (is_tip()) { return tick_result_t::finished; }

  auto valid_teams        = get_tip_bitset();
  auto assigned_team_temp = _assigned_team + 1;

  if (assigned_team_temp >= valid_teams.size() ||
      !valid_teams[assigned_team_temp]) {
    assign_team_reset();
    if (is_cherry()) { return tick_result_t::finished; }
    return tick_children();
  }
  assign_team(assigned_team_temp);
  return tick_result_t::success;
}

auto single_node_t::tick_children() -> tick_result_t {

  if (children().right.is_win()) {
    auto trr = right_child().tick();
    if (trr == tick_result_t::success) { return tick_result_t::success; }
  }

  if (children().left.is_win()) {
    auto trl = left_child().tick();
    if (trl == tick_result_t::success) { return tick_result_t::success; }
  }

  return tick_result_t::finished;
}

auto single_node_t::valid() const -> bool {
  /* The != here is to xor the values. We need exactly ONE of them to be true.
   */

  if (is_tip()) { return true; }
  if (is_cherry()) { return true; }

  size_t lteam =
      children().left.is_win() ? left_child().winner() : left_child().loser();

  size_t rteam = children().right.is_win() ? right_child().winner()
                                           : right_child().loser();

  if ((lteam == _assigned_team) != (rteam == _assigned_team)) {
    return left_child().valid() && right_child().valid();
  }
  return false;
}

void single_node_t::dump_state_graphviz(
    std::ostream                                                &os,
    const std::function<std::string(const single_node_t &)>     &node_attr_func,
    const std::function<std::string(const tournament_edge_t &)> &edge_attr_func)
    const {

  if (!is_tip()) {
    if (children().left.is_win()) {
      left_child().dump_state_graphviz(os, node_attr_func, edge_attr_func);
    }
    if (children().right.is_win()) {
      right_child().dump_state_graphviz(os, node_attr_func, edge_attr_func);
    }
  }

  os << internal_label() << node_attr_func(*this) << "\n";

  if (!is_tip()) {
    os << children().left->get_internal_label() << " -> " << internal_label();
    os << edge_attr_func(children().left) << "\n";

    os << children().right->get_internal_label() << " -> " << internal_label();
    os << edge_attr_func(children().right) << "\n";
  }
}

void single_node_t::debug_graphviz(std::ostream &os) const {
  auto node_attr_func = [](const single_node_t &n) -> std::string {
    std::ostringstream oss;
    oss << "[label=";
    if (n.is_tip()) {
      oss << "\"" << n.team().label << "|" << n.get_internal_label() << "|"
          << n.get_team_index() << "\" ";
    } else {
      oss << "\"" << n.winner() << "\" ";
    }
    if (n.valid()) {
      oss << ",color=green ";
    } else {
      oss << ",color=red ";
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

  os << "digraph {\n";
  os << "node [shape=record]\n";
  dump_state_graphviz(os, node_attr_func, edge_attr_func);
  os << "}";
}
