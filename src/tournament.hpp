#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include <Eigen/Dense>
#include <Eigen/src/Core/Matrix.h>
#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

class tournament_node_t;

class tournament_edge_t {
public:
  enum edge_type_t {
    win,
    loss,
  };

  /* Constructors */
  tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_t::win} {}
  tournament_edge_t(std::unique_ptr<tournament_node_t> &&node,
                    edge_type_t edge_type)
      : _node{std::move(node)}, _edge_type{edge_type} {}
  tournament_edge_t(tournament_node_t *node, edge_type_t edge_type)
      : _node{node}, _edge_type{edge_type} {}

  /* Operators */
  tournament_node_t &operator*() { return *_node; }
  const tournament_node_t *operator->() const { return _node.get(); }
  tournament_node_t *operator->() { return _node.get(); }
  operator bool() const { return _node != nullptr; }

  /* Attributes */
  bool empty() const { return _node == nullptr; }

private:
  /* Data Members */
  std::unique_ptr<tournament_node_t> _node;
  edge_type_t _edge_type;
};

struct tournament_children_t {
  tournament_edge_t left;
  tournament_edge_t right;
};

struct team_t {
  std::string label;
  size_t index;
};

class tournament_node_t {
public:
  tournament_node_t() : _children{team_t()} {}
  tournament_node_t(team_t t) : _children{t} {}
  tournament_node_t(std::string team_name) : _children{team_t{team_name, 0}} {}
  tournament_node_t(tournament_children_t &&c) : _children{std::move(c)} {}

  tournament_node_t(tournament_edge_t &&e1, tournament_edge_t &&e2)
      : tournament_node_t{tournament_children_t{std::move(e1), std::move(e2)}} {
  }

  tournament_node_t(std::unique_ptr<tournament_node_t> &&leftc,
                    tournament_edge_t::edge_type_t leftt,
                    std::unique_ptr<tournament_node_t> &&rightc,
                    tournament_edge_t::edge_type_t rightt)
      : tournament_node_t{tournament_edge_t{std::move(leftc), leftt},
                          tournament_edge_t{std::move(rightc), rightt}} {}

  tournament_node_t(std::unique_ptr<tournament_node_t> &&left,
                    std::unique_ptr<tournament_node_t> &&right)
      : tournament_node_t{std::move(left), tournament_edge_t::win,
                          std::move(right), tournament_edge_t::win} {}

  tournament_node_t(const tournament_children_t &) = delete;
  tournament_node_t(const tournament_edge_t &,
                    const tournament_edge_t &) = delete;
  tournament_node_t(const std::unique_ptr<tournament_node_t> &,
                    tournament_edge_t::edge_type_t,
                    const std::unique_ptr<tournament_node_t> &,
                    tournament_edge_t::edge_type_t) = delete;
  tournament_node_t(const std::unique_ptr<tournament_node_t> &,
                    const std::unique_ptr<tournament_node_t> &) = delete;

  bool is_tip() const { return std::holds_alternative<team_t>(_children); }

  size_t tip_count() const {
    if (is_tip())
      return 1;
    auto &children = std::get<tournament_children_t>(_children);
    return children.left->tip_count() + children.right->tip_count();
  }

  size_t relabel_indicies(size_t index) {
    if (is_tip()) {
      team().index = index;
      return index + 1;
    }
    index = children().left->relabel_indicies(index);
    index = children().right->relabel_indicies(index);
    return index;
  }

  void label_map(std::vector<std::pair<std::string, size_t>> &lm) const {
    if (is_tip()) {
      lm.emplace_back(team().label, team().index);
      return;
    }
    children().left->label_map(lm);
    children().right->label_map(lm);
  }

  void relabel_tips(const std::vector<std::string> labels) {
    if (is_tip()) {
      team().label = labels[team().index];
      return;
    }
    children().left->relabel_tips(labels);
    children().right->relabel_tips(labels);
  }

private:
  inline const tournament_children_t &children() const {
    return std::get<tournament_children_t>(_children);
  }
  inline tournament_children_t &children() {
    return std::get<tournament_children_t>(_children);
  }
  inline const team_t &team() const { return std::get<team_t>(_children); }
  inline team_t &team() { return std::get<team_t>(_children); }

  /* Data Members */
  std::variant<tournament_children_t, team_t> _children;
};

class tournament_t {
public:
  tournament_t()
      : _head{
            tournament_edge_t{new tournament_node_t, tournament_edge_t::win},
            tournament_edge_t{new tournament_node_t, tournament_edge_t::win}} {
    relabel_indicies();
  }

  size_t tip_count() const { return _head.tip_count(); }
  void reset_win_probs(const Eigen::MatrixXd wp) {
    auto tipc = static_cast<long int>(tip_count());

    if (tipc == wp.rows() && tipc == wp.cols()) {
      _win_probs = wp;
    } else {
      throw std::runtime_error("Matrix is the wrong size for the tournament");
    }
  }

  void relabel_indicies() { _head.relabel_indicies(0); }
  void relabel_tips(const std::vector<std::string> &labels) {
    if (tip_count() > labels.size()) {
      throw std::runtime_error("Labels vector is too small");
    }
    _head.relabel_tips(labels);
  }

  std::vector<std::pair<std::string, size_t>> label_map() {
    relabel_indicies();
    std::vector<std::pair<std::string, size_t>> lm;
    _head.label_map(lm);
    return lm;
  }

private:
  tournament_node_t _head;
  Eigen::MatrixXd _win_probs;
};

#endif
