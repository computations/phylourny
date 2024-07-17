#pragma once

#include "mcmc.hpp"
#include "program_options.hpp"
#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <optional>

struct result_t {
  vector_t                win_prob;
  params_t                params;
  std::optional<matrix_t> prob_matrix;
  double                  llh;
};
auto operator<<(std::ostream &os, const result_t &r) -> std::ostream &;

class results_t {
public:
  results_t() = delete;
  results_t(const std::vector<std::string> &t, const team_name_map_t &nm) :
      _bracket_teams{t}, _team_name_map{nm} {
    init();
  }

  results_t &set_run_type(const run_mode_e &rt) {
    _run_type = rt;
    return *this;
  }
  results_t &add_file_output(const std::filesystem::path &prefix);
  results_t &enable_memory_save();

  void   add_result(result_t &&r);
  size_t sample_count() const { return _sample_count; }

private:
  void write_result_to_outfiles(const result_t &r);
  void write_params_line(const result_t &r);
  void write_probs_line(const result_t &r);

  inline std::filesystem::path
  params_filename(const std::filesystem::path &prefix) {
    auto tmp = prefix;
    tmp += ".";
    tmp += describe_run_type(_run_type.value());
    tmp += ".samples.params.csv";
    return tmp;
  }

  inline std::filesystem::path
  probs_filename(const std::filesystem::path &prefix) {
    auto tmp = prefix;
    tmp += ".";
    tmp += describe_run_type(_run_type.value());
    tmp += ".samples.win_probs.csv";
    return tmp;
  }

  inline void init() {
    _all_teams.reserve(_team_name_map.size());
    _all_team_index_map.reserve(_team_name_map.size());

    for (const auto &kv : _team_name_map) {
      _all_teams.push_back(kv.first);
      _all_team_index_map.push_back(kv.second);
    }

    _bracket_team_index_map.resize(_bracket_teams.size());
    for (const auto &t : _bracket_teams) {
      _bracket_team_index_map.push_back(_team_name_map.at(t));
    }
  }

  size_t _sample_count = 0;

  std::optional<std::vector<result_t>> _result_list;

  std::optional<run_mode_e> _run_type;

  std::optional<std::ofstream> _params_outfile;
  std::optional<std::ofstream> _probs_outfile;

  std::vector<std::string> _bracket_teams;
  std::vector<std::string> _all_teams;
  team_name_map_t          _team_name_map;
  std::vector<size_t>      _all_team_index_map;
  std::vector<size_t>      _bracket_team_index_map;
};
