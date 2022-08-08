#ifndef MCMC_H
#define MCMC_H

#include "program_options.hpp"
#include <unordered_map>

using team_name_map_t = std::unordered_map<std::string, size_t>;

void mcmc_run(const program_options_t &cli_options);

void compute_tournament(const program_options_t &cli_options);

auto create_name_map(std::vector<std::string> team_names) -> team_name_map_t;
#endif
