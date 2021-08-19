#ifndef __SIMULATION_NODE_HPP__
#define __SIMULATION_NODE_HPP__

#include "tournament_node.hpp"
#include "util.hpp"
#include <random>

class simulation_node_t : public tournament_node_t {
public:
  simulation_node_t()                          = default;
  simulation_node_t(simulation_node_t &&)      = default;
  simulation_node_t(const simulation_node_t &) = default;

  simulation_node_t(const std::shared_ptr<simulation_node_t> &l,
                    const std::shared_ptr<simulation_node_t> &r) :
      tournament_node_t{l, r} {}
  simulation_node_t(const std::shared_ptr<simulation_node_t> &l,
                    tournament_edge_t::edge_type_e            lt,
                    const std::shared_ptr<simulation_node_t> &r,
                    tournament_edge_t::edge_type_e            rt) :
      tournament_node_t{tournament_edge_t{l, lt}, tournament_edge_t{r, rt}} {}

  simulation_node_t(std::string s) : tournament_node_t{s} {}

  ~simulation_node_t() = default;

  size_t winner() const override {
    if (is_tip()) { return team().index; }
    return _assigned_team;
  }

  size_t loser() const override {
    if (is_tip()) { throw std::runtime_error{"Called loser on a tip"}; }

    if (!is_cherry() && (left_child().winner() == left_child().loser() ||
                         right_child().winner() == right_child().loser())) {
      throw std::runtime_error{"Winner and loser is the same for some reason"};
    }

    if (children().left->winner() == _assigned_team) {
      return children().right->winner();
    }
    return children().left->winner();
  }

  vector_t eval(const matrix_t &pmat, size_t tip_count, size_t iters) {
    std::vector<size_t> counts(tip_count, 0);

    clock_tick_t clock = 1;
    reset_clocks();

    random_engine_t random_engine{std::random_device()()};

    for (size_t i = 0; i < iters; i++) {
      clock = simulation_eval(pmat, random_engine, clock);
      clock++;
      counts[winner()] += 1;
    }

    vector_t results(tip_count);

    for (size_t i = 0; i < tip_count; i++) {
      results[i] = static_cast<double>(counts[i]) / static_cast<double>(iters);
    }

    return results;
  }

private:
  size_t simulation_eval(const matrix_t & pmat,
                         random_engine_t &random_engine,
                         clock_tick_t     clock) {
    if (is_tip()) { return team().index; }

    if (clock < _last_eval) { return _assigned_team; }

    auto lclock = left_child().simulation_eval(pmat, random_engine, clock);
    auto rclock = right_child().simulation_eval(pmat, random_engine, clock);

    size_t lteam =
        children().left.is_win() ? left_child().winner() : left_child().loser();

    size_t rteam = children().right.is_win() ? right_child().winner()
                                             : right_child().loser();

    std::bernoulli_distribution d(pmat[lteam][rteam]);

    _assigned_team = d(random_engine) ? lteam : rteam;
    clock          = std::max(lclock, rclock);
    _last_eval     = clock++;
    return clock;
  }

  bool is_cherry() const {
    if (is_tip()) { return false; }
    return left_child().is_tip() && right_child().is_tip();
  }

  simulation_node_t &left_child() {
    return dynamic_cast<simulation_node_t &>(*children().left);
  }

  simulation_node_t &right_child() {
    return dynamic_cast<simulation_node_t &>(*children().right);
  }

  const simulation_node_t &left_child() const {
    return dynamic_cast<const simulation_node_t &>(*children().left);
  }

  const simulation_node_t &right_child() const {
    return dynamic_cast<const simulation_node_t &>(*children().right);
  }

  void reset_clocks() {
    _last_eval = 0;
    if (is_tip()) { return; }
    left_child().reset_clocks();
    right_child().reset_clocks();
  }

  size_t       _assigned_team;
  clock_tick_t _last_eval = 0;
};

#endif