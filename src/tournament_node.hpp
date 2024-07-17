#ifndef TOURNAMENT_NODE_HPP
#define TOURNAMENT_NODE_HPP

#include "util.hpp"
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct team_t {
  std::string label;
  size_t      index{};
};

struct scratchpad_t {
  scratchpad_t() {}
  double fold_l = 0.0;
  double fold_r = 0.0;

  double result = 0.0;

  tip_bitset_t include;
  size_t       eval_index{};
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

  explicit tournament_edge_t() : _node{nullptr}, _edge_type{edge_type_e::win} {}
  explicit tournament_edge_t(const std::shared_ptr<tournament_node_t> &node,
                             edge_type_e edge_type) :
      _node{node}, _edge_type{edge_type} {}
  explicit tournament_edge_t(tournament_node_t *node, edge_type_e edge_type) :
      _node{node}, _edge_type{edge_type} {}

  auto operator*() -> tournament_node_t & { return *_node; }
  auto operator*() const -> const tournament_node_t & { return *_node; }
  auto operator->() -> tournament_node_t * { return _node.get(); }
  auto operator->() const -> const tournament_node_t * { return _node.get(); }
  operator bool() const { return _node != nullptr; }

  [[nodiscard]] auto        empty() const -> bool { return _node == nullptr; }
  [[nodiscard]] inline auto is_win() const -> bool {
    return _edge_type == edge_type_e::win;
  }
  [[nodiscard]] inline auto is_loss() const -> bool { return !is_win(); }
  [[nodiscard]] auto        is_simple() const -> bool;

  [[nodiscard]] inline auto eval(const matrix_t &pmatrix,
                                 size_t          tip_count) const -> vector_t;

  [[nodiscard]] inline auto single_eval(const matrix_t   &pmatrix,
                                        size_t            eval_index,
                                        std::vector<bool> exclude) const
      -> double;

private:
  /* Data Members */
  std::shared_ptr<tournament_node_t> _node;
  edge_type_e                        _edge_type;
};

struct match_parameters_t {
  tournament_edge_t left;
  tournament_edge_t right;
  uint64_t          bestof = 1;

  [[nodiscard]] auto is_simple() const -> bool {
    return left.is_simple() && right.is_simple();
  }
};

class tournament_node_t {
public:
  explicit tournament_node_t() : _children{team_t()} {}
  explicit tournament_node_t(const team_t &t) : _children{t} {}

  explicit tournament_node_t(const std::string &team_name) :
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

  [[nodiscard]] auto is_tip() const -> bool;
  [[nodiscard]] auto tip_count() const -> size_t;
  [[nodiscard]] auto is_member(size_t index) const -> bool;

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
  auto relabel_indicies(size_t index) -> size_t;

  /**
   * Determines if the current node can be computed using a simple method.
   */
  [[nodiscard]] auto is_simple() const -> bool;

  void store_node_results(std::unordered_map<std::string, vector_t>& res_map);

  void reset_saved_evals();

  auto        eval(const matrix_t &pmatrix, size_t tip_count) -> vector_t;
  static auto fold(const vector_t &x,
                   const vector_t &y,
                   uint64_t        bestof,
                   const matrix_t &pmatrix) -> vector_t;

  auto               set_tip_bitset(size_t tip_count) -> tip_bitset_t;
  [[nodiscard]] auto get_tip_bitset() const -> tip_bitset_t {
    return _tip_bitset;
  }

  virtual void assign_internal_labels() { assign_internal_labels(0); }

  virtual auto assign_internal_labels(size_t index) -> size_t {
    if (_internal_label.empty()) { _internal_label = compute_base26(index++); }
    if (!is_tip()) {
      index = children().left->assign_internal_labels(index);
      index = children().right->assign_internal_labels(index);
    }
    return index;
  }

  [[nodiscard]] auto get_internal_label() const -> std::string {
    return _internal_label;
  }

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

  [[nodiscard]] auto get_display_label() const -> std::string {
    if (is_tip() && !team().label.empty()) { return team().label; }
    return _internal_label;
  }

  [[nodiscard]] auto get_memoized_values() const -> vector_t {
    return _memoized_values;
  }

  [[nodiscard]] auto get_scratch_pad() const -> scratchpad_t {
    return _scratchpad;
  }

  [[nodiscard]] auto get_team_index() const -> size_t { return team().index; }

  void debug_graphviz(std::ostream &os) const {
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

  [[nodiscard]] virtual auto winner() const -> size_t {
    throw std::runtime_error{"Not implemented"};
  }

  [[nodiscard]] virtual auto loser() const -> size_t {
    throw std::runtime_error{"Not implemented"};
  }

  void set_bestof(const std::function<size_t(size_t)> &b_func, size_t depth);

  void set_bestof(size_t b) {
    set_bestof([b](size_t) -> size_t { return b; }, 0);
  }

  [[nodiscard]] auto internal_label() const -> const std::string & {
    return _internal_label;
  }

  size_t count_tips() const { return count_tips(0); }

  size_t count_tips(size_t cur) const {
    if (is_tip()) { return cur + 1; }
    cur = children().left->count_tips(cur);
    cur = children().right->count_tips(cur);
    return cur;
  }

protected:
  [[nodiscard]] inline auto children() const -> const match_parameters_t & {
    return std::get<match_parameters_t>(_children);
  }
  inline auto children() -> match_parameters_t & {
    return std::get<match_parameters_t>(_children);
  }
  [[nodiscard]] inline auto team() const -> const team_t & {
    return std::get<team_t>(_children);
  }
  inline auto team() -> team_t & { return std::get<team_t>(_children); }

private:
  [[nodiscard]] auto eval_saved() const -> bool {
    return !_memoized_values.empty();
  }

  [[nodiscard]] auto team_count() const -> size_t { return _tip_bitset.size(); }

  /* Data Members */
  std::variant<match_parameters_t, team_t> _children;
  vector_t                                 _memoized_values;
  tip_bitset_t                             _tip_bitset;
  std::string                              _internal_label;
  scratchpad_t                             _scratchpad;
};

#endif // __TOURNAMENT_NODE_HPP__
