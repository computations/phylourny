#ifndef __TOURNAMENT_HPP__
#define __TOURNAMENT_HPP__
#include <memory>

class tournament_node_t;

class tournament_node_ptr_t {
public:
  tournament_node_ptr_t() : _node{nullptr}, _win{true} {}
  tournament_node_ptr_t(std::shared_ptr<tournament_node_t> node, bool win)
      : _node{node}, _win{win} {}

  tournament_node_t &operator*() { return *_node; }
  tournament_node_t *operator->() { return _node.get(); }
  operator bool() const { return _node != nullptr; }

  bool empty() const { return _node == nullptr; }

private:
  std::shared_ptr<tournament_node_t> _node;
  bool _win;
};

class tournament_node_t {
public:
  tournament_node_t() : _lnode{}, _rnode{} {}
  tournament_node_t(std::shared_ptr<tournament_node_t> lnode, bool lwin,
                    std::shared_ptr<tournament_node_t> rnode, bool rwin)
      : _lnode{lnode, lwin}, _rnode{rnode, rwin} {}

  size_t count() {
    if (is_tip()) {
      return 1;
    }

    return _lnode->count() + _rnode->count();
  }

private:
  bool is_tip() const { return !(_lnode && _rnode); }

  /* data members */
  tournament_node_ptr_t _lnode;
  tournament_node_ptr_t _rnode;
};

#endif
