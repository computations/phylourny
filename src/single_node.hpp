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
  explicit single_node_t()                      = default;
  explicit single_node_t(single_node_t &&)      = default;
  explicit single_node_t(const single_node_t &) = default;
  explicit single_node_t(const std::shared_ptr<single_node_t> &l,
                         const std::shared_ptr<single_node_t> &r) :
      tournament_node_t{l, r} {}
  explicit single_node_t(const std::shared_ptr<single_node_t> &l,
                         tournament_edge_t::edge_type_e        lt,
                         const std::shared_ptr<single_node_t> &r,
                         tournament_edge_t::edge_type_e        rt) :
      tournament_node_t{tournament_edge_t{l, lt}, tournament_edge_t{r, rt}} {}

  explicit single_node_t(const std::string &s) : tournament_node_t{s} {}

  ~single_node_t() override = default;

  auto eval(const matrix_t &pmatrix, size_t tip_count) -> vector_t;
  auto eval_debug(const matrix_t &pmatrix,
                  size_t          tip_count,
                  const std::string &) -> vector_t;
  void store_node_results(std::unordered_map<std::string, vector_t> &res_map) {
    (void)(res_map);
    throw std::runtime_error{"Unimplemneted function"};
  }

  [[nodiscard]] auto winner() const -> size_t override {
    return _assigned_team;
  }

  [[nodiscard]] auto loser() const -> size_t override {
    return returned_team(false);
  }

  [[nodiscard]] auto returned_team(bool winner) const -> size_t {
    if (winner) { return _assigned_team; }

    if (is_tip()) { throw std::runtime_error{"Called team on a tip"}; }

    if (left_team() == _assigned_team) { return right_team(); }

    return left_team();
  }

  void init_assigned_teams();

  auto tick() -> tick_result_t;

  [[nodiscard]] auto valid() const -> bool;
  [[nodiscard]] auto is_cherry() const -> bool;

  void dump_state_graphviz(
      std::ostream                                            &os,
      const std::function<std::string(const single_node_t &)> &node_attr_func,
      const std::function<std::string(const tournament_edge_t &)>
          &edge_attr_func) const;

  void debug_graphviz(std::ostream &os) const;

private:
  auto tick_children() -> tick_result_t;

  auto left_child() -> single_node_t & {
    return dynamic_cast<single_node_t &>(*children().left);
  }

  auto right_child() -> single_node_t & {
    return dynamic_cast<single_node_t &>(*children().right);
  }

  [[nodiscard]] auto left_child() const -> const single_node_t & {
    return dynamic_cast<const single_node_t &>(*children().left);
  }

  [[nodiscard]] auto right_child() const -> const single_node_t & {
    return dynamic_cast<const single_node_t &>(*children().right);
  }

  [[nodiscard]] auto left_team() const -> size_t {
    return left_child().returned_team(children().left.is_win());
  }

  [[nodiscard]] auto right_team() const -> size_t {
    return right_child().returned_team(children().right.is_win());
  }

  auto single_eval(const matrix_t &pmatrix, bool winner) -> double;

  void assign_team(size_t t) { _assigned_team = t; }
  void assign_team_reset();
  auto internal_eval(const matrix_t &pmatrix, size_t tip_count) -> double;

  size_t _assigned_team = 0;

  double _saved_val = 0;
};

#endif
