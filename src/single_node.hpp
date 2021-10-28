#ifndef SINGLE_NODE_HPP
#define SINGLE_NODE_HPP

#include "tournament_node.hpp"
#include "util.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>

enum class tick_result_t {
  failure,
  success,
  carry,
  finished,
};

class single_node_t : public tournament_node_t {
public:
  single_node_t()                      = default;
  single_node_t(single_node_t &&)      = default;
  single_node_t(const single_node_t &) = default;
  single_node_t(const std::shared_ptr<single_node_t> &l,
                const std::shared_ptr<single_node_t> &r) :
      tournament_node_t{l, r} {}
  single_node_t(const std::shared_ptr<single_node_t> &l,
                tournament_edge_t::edge_type_e        lt,
                const std::shared_ptr<single_node_t> &r,
                tournament_edge_t::edge_type_e        rt) :
      tournament_node_t{tournament_edge_t{l, lt}, tournament_edge_t{r, rt}} {}

  single_node_t(std::string s) : tournament_node_t{s} {}

  ~single_node_t() override = default;

  vector_t eval(const matrix_t &pmatrix, size_t tip_count);
  vector_t
  eval_debug(const matrix_t &pmatrix, size_t tip_count, const std::string &);

  size_t winner() const override { return _assigned_team; }

  size_t loser() const override { return returned_team(false); }

  size_t returned_team(bool winner) const {
    if (winner) { return _assigned_team; }

    if (is_tip()) { throw std::runtime_error{"Called team on a tip"}; }

    if (left_team() == _assigned_team) { return right_team(); }

    return left_team();
  }

  void init_assigned_teams();

  tick_result_t tick();

  bool valid() const;
  bool is_cherry() const;

  void dump_state_graphviz(
      std::ostream &                                           os,
      const std::function<std::string(const single_node_t &)> &node_attr_func,
      const std::function<std::string(const tournament_edge_t &)>
          &edge_attr_func) const;

  void debug_graphviz(std::ostream &os) const;

private:
  tick_result_t tick_children();

  single_node_t &left_child() {
    return dynamic_cast<single_node_t &>(*children().left);
  }

  single_node_t &right_child() {
    return dynamic_cast<single_node_t &>(*children().right);
  }

  const single_node_t &left_child() const {
    return dynamic_cast<const single_node_t &>(*children().left);
  }

  const single_node_t &right_child() const {
    return dynamic_cast<const single_node_t &>(*children().right);
  }

  size_t left_team() const {
    return left_child().returned_team(children().left.is_win());
  }

  size_t right_team() const {
    return right_child().returned_team(children().right.is_win());
  }

  double single_eval(const matrix_t &pmatrix, bool winner);

  void   assign_team(size_t t) { _assigned_team = t; }
  void   assign_team_reset();
  double internal_eval(const matrix_t &pmatrix, size_t tip_count);

  size_t _assigned_team = 0;

  double _saved_val = 0;
};

#endif
