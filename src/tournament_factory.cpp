#include "simulation_node.hpp"
#include "single_node.hpp"
#include "tournament_factory.hpp"
#include "tournament_node.hpp"

/** @defgroup tournament_factory_templates Tournament factory template functions
 * @{
 */

template <typename T>
auto tournament_node_factory_template(size_t sub_tourny_size) -> std::shared_ptr<T> {
  if (sub_tourny_size == 1) { return std::shared_ptr<T>{new T{}}; }
  if (sub_tourny_size % 2 != 0) {
    throw std::runtime_error(
        "Tournament node factory only accepts powers of 2");
  }

  std::vector<std::unique_ptr<T>> nodes;
  nodes.reserve(sub_tourny_size);
  for (size_t i = 0; i < sub_tourny_size; ++i) { nodes.emplace_back(new T{}); }

  while (nodes.size() != 1) {
    size_t                          cur_size = nodes.size() / 2;
    std::vector<std::unique_ptr<T>> tmp_nodes;
    tmp_nodes.reserve(cur_size);
    for (size_t i = 0, j = 1; j < nodes.size(); i += 2, j += 2) {
      tmp_nodes.emplace_back(new T{std::move(nodes[i]), std::move(nodes[j])});
    }
    std::swap(tmp_nodes, nodes);
  }

  return std::move(nodes[0]);
}

template <typename T>
auto tournament_factory_template(size_t tourny_size) -> tournament_t<T> {
  if (tourny_size % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }

  tournament_t<T> t{
      new T{tournament_node_factory_template<T>(tourny_size / 2),
            tournament_node_factory_template<T>(tourny_size / 2)}};
  t.relabel_indicies();
  return t;
}

template <typename T>
auto tournament_factory_template(size_t tourny_size_l,
                                            size_t tourny_size_r) -> tournament_t<T> {
  if (tourny_size_r % 2 != 0 || tourny_size_l % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }

  tournament_t<T> t{new T{tournament_node_factory_template<T>(tourny_size_l),
                          tournament_node_factory_template<T>(tourny_size_r)}};
  t.relabel_indicies();
  return t;
}

/** @} */

auto
tournament_node_factory(size_t sub_tourny_size) -> std::shared_ptr<tournament_node_t> {
  return tournament_node_factory_template<tournament_node_t>(sub_tourny_size);
}

auto
tournament_node_factory_single(size_t sub_tourny_size) -> std::shared_ptr<single_node_t> {
  return tournament_node_factory_template<single_node_t>(sub_tourny_size);
}

auto
tournament_node_factory_simulation(size_t sub_tourny_size) -> std::shared_ptr<simulation_node_t> {
  return tournament_node_factory_template<simulation_node_t>(sub_tourny_size);
}

auto tournament_factory(size_t tourny_size) -> tournament_t<tournament_node_t> {
  return tournament_factory_template<tournament_node_t>(tourny_size);
}

auto tournament_factory_single(size_t tourny_size) -> tournament_t<single_node_t> {
  return tournament_factory_template<single_node_t>(tourny_size);
}

auto
tournament_factory_simulation(size_t tourny_size) -> tournament_t<simulation_node_t> {
  return tournament_factory_template<simulation_node_t>(tourny_size);
}

auto tournament_factory(size_t tourny_size_l,
                                                   size_t tourny_size_r) -> tournament_t<tournament_node_t> {
  return tournament_factory_template<tournament_node_t>(tourny_size_l,
                                                        tourny_size_r);
}

auto tournament_factory_single(size_t tourny_size_l,
                                                      size_t tourny_size_r) -> tournament_t<single_node_t> {
  return tournament_factory_template<single_node_t>(tourny_size_l,
                                                    tourny_size_r);
}

auto
tournament_factory_simulation(size_t tourny_size_l, size_t tourny_size_r) -> tournament_t<simulation_node_t> {
  return tournament_factory_template<simulation_node_t>(tourny_size_l,
                                                        tourny_size_r);
}

auto
tournament_factory(const std::vector<std::string> &team_labels) -> tournament_t<tournament_node_t> {
  auto t = tournament_factory_template<tournament_node_t>(team_labels.size());
  t.relabel_tips(team_labels);
  return t;
}

auto
tournament_factory_single(const std::vector<std::string> &team_labels) -> tournament_t<single_node_t> {
  auto t = tournament_factory_template<single_node_t>(team_labels.size());
  t.relabel_tips(team_labels);
  return t;
}

auto
tournament_factory_simulation(const std::vector<std::string> &team_labels) -> tournament_t<simulation_node_t> {
  auto t = tournament_factory_template<simulation_node_t>(team_labels.size());
  t.relabel_tips(team_labels);
  return t;
}
