#include "tournament_factory.hpp"

std::shared_ptr<tournament_node_t>
tournament_node_factory(size_t sub_tourny_size) {
  if (sub_tourny_size == 1) {
    return std::shared_ptr<tournament_node_t>{new tournament_node_t{}};
  }
  if (sub_tourny_size % 2 != 0) {
    throw std::runtime_error(
        "Tournament node factory only accepts powers of 2");
  }

  std::vector<std::unique_ptr<tournament_node_t>> nodes;
  nodes.reserve(sub_tourny_size);
  for (size_t i = 0; i < sub_tourny_size; ++i) {
    nodes.emplace_back(new tournament_node_t{});
  }

  while (nodes.size() != 1) {
    size_t                                          cur_size = nodes.size() / 2;
    std::vector<std::unique_ptr<tournament_node_t>> tmp_nodes;
    tmp_nodes.reserve(cur_size);
    for (size_t i = 0, j = 1; j < nodes.size(); i += 2, j += 2) {
      tmp_nodes.emplace_back(
          new tournament_node_t{std::move(nodes[i]), std::move(nodes[j])});
    }
    std::swap(tmp_nodes, nodes);
  }

  return std::move(nodes[0]);
}

tournament_t tournament_factory(size_t tourny_size) {
  if (tourny_size % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }

  tournament_t t{
      new tournament_node_t{tournament_node_factory(tourny_size / 2),
                            tournament_node_factory(tourny_size / 2)}};
  t.relabel_indicies();
  return t;
}

tournament_t tournament_factory(size_t tourny_size_l, size_t tourny_size_r) {
  if (tourny_size_r % 2 != 0 || tourny_size_l % 2 != 0) {
    throw std::runtime_error("Tournament factory only accepts powers of 2");
  }

  tournament_t t{new tournament_node_t{tournament_node_factory(tourny_size_l),
                                       tournament_node_factory(tourny_size_r)}};
  t.relabel_indicies();
  return t;
}

tournament_t tournament_factory(const std::vector<std::string> &team_labels) {
  auto t = tournament_factory(team_labels.size());
  t.relabel_tips(team_labels);
  return t;
}
