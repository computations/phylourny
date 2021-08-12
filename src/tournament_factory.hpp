#ifndef __TOURNAMENT_FACTORY_HPP__
#define __TOURNAMENT_FACTORY_HPP__

#include "simulation_node.hpp"
#include "single_node.hpp"
#include "tournament.hpp"
#include "tournament_node.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

template <typename T> class tournament_t;

tournament_t<tournament_node_t> tournament_factory(size_t tourny_size);
tournament_t<tournament_node_t>
tournament_factory(const std::vector<std::string> &);
tournament_t<tournament_node_t> tournament_factory(size_t tourny_size_l,
                                                   size_t tourny_size_r);

std::shared_ptr<tournament_node_t>
tournament_node_factory(size_t sub_tourny_size);

tournament_t<single_node_t> tournament_factory_single(size_t tourny_size);
tournament_t<single_node_t>
tournament_factory_single(const std::vector<std::string> &);
tournament_t<single_node_t> tournament_factory_single(size_t tourny_size_l,
                                                      size_t tourny_size_r);

std::shared_ptr<single_node_t>
tournament_node_factory_single(size_t sub_tourny_size);

tournament_t<simulation_node_t>
tournament_factory_simulation(size_t tourny_size);
tournament_t<simulation_node_t>
tournament_factory_simulation(const std::vector<std::string> &);
tournament_t<simulation_node_t>
tournament_factory_simulation(size_t tourny_size_l, size_t tourny_size_r);

std::shared_ptr<simulation_node_t>
tournament_node_factory_simulation(size_t sub_tourny_size);
#endif
