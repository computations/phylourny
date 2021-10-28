#ifndef __TOURNAMENT_NODE_HPP__
#define __TOURNAMENT_NODE_HPP__

#include "util.hpp"
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sul/dynamic_bitset.hpp>
#include <variant>
#include <vector>

struct team_t {
  std::string label;
  size_t      index;
};

struct scratchpad_t {
  double fold_l = 0.0;
  double fold_r = 0.0;

  double result = 0.0;

  tip_bitset_t include;
  size_t       eval_index;
};

class tournament_node_t;

/**
 * A class representing the edge of a tournament. It has 2 "colors", win or
 * lose, which indicates what kind of edge it is. If it is a win edge, then
 * winner of the node travels along this edge to the adjacent node.
 *
 * This edge is directed. This is to say, it isn't a pair of nodes, but only
 * contains the end node. Additionally, the tournament arcs are reversed. This
 * is to say, normally the teams start at the tips and advance in. For the
 * purposes of calculating the result, we reverse all the arcs. So, what is
 * pointed at by this edge is the "child" of some other node.
 *
 * In summary, this is a shared pointer with a color, indicating what team to
 * get from the child.
 */
class tournament_edge_t {
public:
  enum struct edge_type_e {
    win,
    loss,
  };

  tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_e::win} {}
  tournament_edge_t(const std::shared_ptr<tournament_node_t> &node,
                    edge_type_e                               edge_type) :
      _node{std::move(node)}, _edge_type{edge_type} {}
  tournament_edge_t(tournament_node_t *node, edge_type_e edge_type) :
      _node{node}, _edge_type{edge_type} {}

  tournament_node_t &      operator*() { return *_node; }
  const tournament_node_t &operator*() const { return *_node; }
  tournament_node_t *      operator->() { return _node.get(); }
  const tournament_node_t *operator->() const { return _node.get(); }
                           operator bool() const { return _node != nullptr; }

  bool        empty() const { return _node == nullptr; }
  inline bool is_win() const { return _edge_type == edge_type_e::win; }
  inline bool is_loss() const { return !is_win(); }
  bool        is_simple() const;

  inline vector_t eval(const matrix_t &pmatrix, size_t tip_count) const;

  inline double single_eval(const matrix_t &  pmatrix,
                            size_t            eval_index,
                            std::vector<bool> exclude) const;

private:
  /* Data Members */
  std::shared_ptr<tournament_node_t> _node;
  edge_type_e                        _edge_type;
};

struct match_parameters_t {
  tournament_edge_t left;
  tournament_edge_t right;
  uint64_t          bestof = 1;

  bool is_simple() const { return left.is_simple() && right.is_simple(); }
};

class tournament_node_t {
public:
  tournament_node_t() : _children{team_t()} {}
  explicit tournament_node_t(team_t t) : _children{t} {}
  explicit tournament_node_t(std::string team_name) :
      _children{team_t{team_name, 0}} {}

  explicit tournament_node_t(const match_parameters_t &c) : _children{c} {}
  explicit tournament_node_t(const tournament_edge_t &l,
                             const tournament_edge_t &r) :
      tournament_node_t{match_parameters_t{l, r}} {}
  explicit tournament_node_t(const std::shared_ptr<tournament_node_t> &l,
                             tournament_edge_t::edge_type_e            lt,
                             const std::shared_ptr<tournament_node_t> &r,
                             tournament_edge_t::edge_type_e            rt) :
      tournament_node_t{tournament_edge_t{l, lt}, tournament_edge_t{r, rt}} {}
  explicit tournament_node_t(const std::shared_ptr<tournament_node_t> &l,
                             const std::shared_ptr<tournament_node_t> &r) :
      tournament_node_t{l,
                        tournament_edge_t::edge_type_e::win,
                        r,
                        tournament_edge_t::edge_type_e::win} {}

  virtual ~tournament_node_t() = default;

  bool   is_tip() const;
  size_t tip_count() const;
  bool   is_member(size_t index) const;

  /**
   * Create a map of the team labels to team indexes.
   *
   * @param[out] lm The function will just place the label index pairs at the
   * back of this vector, the vector should be empty when this is called.
   */
  void label_map(std::vector<std::pair<std::string, size_t>> &lm) const;

  /**
   * Label the tips using the vector as a map from team index to team label.
   *
   * @param labels Map of team indices to team labels.
   */
  void relabel_tips(const std::vector<std::string> &labels);

  /**
   * Relabel the team indices starting at `index`. Traverses the tree in a
   * preorder fashion, descending the left child first.
   */
  size_t relabel_indicies(size_t index);

  /**
   * Determines if the current node can be computed using a simple method.
   */
  bool is_simple() const;

  void reset_saved_evals();

  vector_t eval(const matrix_t &pmatrix, size_t tip_count);
  vector_t fold(const vector_t &x,
                const vector_t &y,
                uint64_t        bestof,
                const matrix_t &pmatrix) const;

  tip_bitset_t set_tip_bitset(size_t tip_count);
  tip_bitset_t get_tip_bitset() const { return _tip_bitset; };

  virtual void assign_internal_labels() { assign_internal_labels(0); }

  virtual size_t assign_internal_labels(size_t index) {
    if (_internal_label.empty()) { _internal_label = compute_base26(index++); }
    if (!is_tip()) {
      index = children().left->assign_internal_labels(index);
      index = children().right->assign_internal_labels(index);
    }
    return index;
  }

  std::string get_internal_label() const { return _internal_label; }

  void dump_state_graphviz(
      std::ostream &os,
      const std::function<std::string(const tournament_node_t &)>
          &node_attr_func,
      const std::function<std::string(const tournament_edge_t &)>
          &edge_attr_func) const {

    if (!is_tip()) {
      if (children().left.is_win()) {
        children().left->dump_state_graphviz(
            os, node_attr_func, edge_attr_func);
      }
      if (children().right.is_win()) {
        children().right->dump_state_graphviz(
            os, node_attr_func, edge_attr_func);
      }
    }

    os << _internal_label << node_attr_func(*this) << "\n";

    if (!is_tip()) {
      os << children().left->get_internal_label() << " -> " << _internal_label;
      os << edge_attr_func(children().left) << "\n";

      os << children().right->get_internal_label() << " -> " << _internal_label;
      os << edge_attr_func(children().right) << "\n";
    }
  }

  std::string get_display_label() const {
    if (is_tip() && !team().label.empty()) { return team().label; }
    return _internal_label;
  }

  vector_t get_memoized_values() const { return _memoized_values; }

  scratchpad_t get_scratch_pad() const { return _scratchpad; }

  size_t get_team_index() const { return team().index; }

  void debug_graphviz(std::ostream &os) {
    auto node_attr_func = [](const tournament_node_t &n) -> std::string {
      std::ostringstream oss;
      oss << "[label=";
      if (n.is_tip()) {
        oss << "\"" << n.team().label << "|" << n.get_internal_label() << "|"
            << n.get_team_index() << "\" ";
      } else {
        auto sp = n.get_scratch_pad();
        //        if (!n.is_simple()) {
        oss << "\"" << n.get_display_label() << "|" << sp.fold_l << "|"
            << sp.fold_r << "|" << sp.result << "|" << sp.eval_index << "|"
            << sp.include.to_string() << "|" << n._tip_bitset.to_string()
            << "\" ";
        /*
      } else {
        oss << sp.result << " ";
        oss << "style = filled ";
      }
      */
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

  virtual size_t winner() const { throw std::runtime_error{"Not implemented"}; }

  virtual size_t loser() const { throw std::runtime_error{"Not implemented"}; }

  void set_bestof(const std::function<size_t(size_t)> &b_func, size_t depth);

  void set_bestof(size_t b) {
    set_bestof([b](size_t) -> size_t { return b; }, 0);
  }

  const std::string &internal_label() const { return _internal_label; }

protected:
  inline const match_parameters_t &children() const {
    return std::get<match_parameters_t>(_children);
  }
  inline match_parameters_t &children() {
    return std::get<match_parameters_t>(_children);
  }
  inline const team_t &team() const { return std::get<team_t>(_children); }
  inline team_t &      team() { return std::get<team_t>(_children); }

private:
  bool eval_saved() const { return _memoized_values.size() != 0; }

  size_t team_count() const { return _tip_bitset.size(); }

  /* Data Members */
  std::variant<match_parameters_t, team_t> _children;
  vector_t                                 _memoized_values;
  tip_bitset_t                             _tip_bitset;
  std::string                              _internal_label;
  scratchpad_t                             _scratchpad;
};

#endif // __TOURNAMENT_NODE_HPP__
