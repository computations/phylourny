#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include <Eigen/Dense>
#include <exception>
#include <limits>
#include <memory>
#include <vector>

class tournament_node_t;

class tournament_edge_t {
public:
  enum edge_type_t {
    win,
    loss,
  };
  tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_t::win} {}
  tournament_edge_t(std::shared_ptr<tournament_node_t> node,
                    edge_type_t edge_type)
      : _node{node}, _edge_type{edge_type} {}
  tournament_edge_t(tournament_node_t *node, edge_type_t edge_type)
      : _node{node}, _edge_type{edge_type} {}

  tournament_node_t &operator*() { return *_node; }
  const tournament_node_t *operator->() const { return _node.get(); }
  tournament_node_t *operator->() { return _node.get(); }
  operator bool() const { return _node != nullptr; }

  bool empty() const { return _node == nullptr; }

private:
  std::shared_ptr<tournament_node_t> _node;
  edge_type_t _edge_type;
};

class tournament_node_t {
public:
  tournament_node_t() : _lnode{}, _rnode{} {}
  tournament_node_t(std::shared_ptr<tournament_node_t> lnode,
                    tournament_edge_t::edge_type_t lwin,
                    std::shared_ptr<tournament_node_t> rnode,
                    tournament_edge_t::edge_type_t rwin)
      : _lnode{lnode, lwin}, _rnode{rnode, rwin} {}
  tournament_node_t(tournament_edge_t lnode, tournament_edge_t rnode)
      : _lnode{lnode}, _rnode{rnode} {}

  size_t tip_count() const {
    if (is_tip()) {
      return 1;
    }

    return _lnode->tip_count() + _rnode->tip_count();
  }

  Eigen::VectorXd eval(const Eigen::MatrixXd &wps) {

    if (is_tip()) {
      Eigen::VectorXd wpv(wps.rows());
      wpv[_index] = 1.0;
      return wpv;
    } else {
      // construct the matrix
      // and then evaluate the matrix
    }
  }

private:
  bool is_tip() const { return !(_lnode && _rnode); }

  /* data members */
  tournament_edge_t _lnode;
  tournament_edge_t _rnode;
  size_t _index;
  // Eigen::VectorXd _wpv;
};

class tournament_t {
public:
  tournament_t()
      : _head{{new tournament_node_t{}, tournament_edge_t::edge_type_t::win},
              {new tournament_node_t{}, tournament_edge_t::edge_type_t::win}} {}

  size_t tip_count() const { return _head.tip_count(); }

  void reset_win_probs(Eigen::MatrixXd wps) {
    long current_tip_count = tip_count();
    if (current_tip_count == wps.rows() && current_tip_count == wps.cols()) {
      _win_probs = std::move(wps);
    } else {
      throw std::runtime_error{
          "Win probability matrix does not fit the current tournament, either "
          "the rows or cols are different"};
    }
  }

  Eigen::VectorXd eval() {
    if (!check_win_probs()) {
      throw std::runtime_error{"Win probabiltiy matrix is not the correct size "
                               "when trying evaluate the tournament"};
    }
  }

private:
  bool check_win_probs() const {
    long current_tip_count = tip_count();
    if (current_tip_count == _win_probs.rows() &&
        current_tip_count == _win_probs.cols()) {
      return false;
    }
    return true;
  }
  tournament_node_t _head;
  Eigen::MatrixXd _win_probs;
};

#endif
