#include "results.hpp"

#include <fstream>

auto operator<<(std::ostream &os, const result_t &r) -> std::ostream & {
  os << "{\"win_prob\": " << to_json(r.win_prob) << ", ";
  os << "\"params\": " << to_json(r.params) << ", ";
  os << "\"llh\": " << std::to_string(r.llh);
  if (r.prob_matrix.has_value()) {
    os << ", ";
    os << "\"matrix\": " << to_json(r.prob_matrix.value());
  }
  os << "}";
  return os;
}

template <typename T>
void write_csv_line(std::ostream              &os,
                    const std::vector<size_t> &team_index_map,
                    const T                   &res,
                    double                     llh) {
  os << std::accumulate(
            std::next(team_index_map.begin()),
            team_index_map.end(),
            std::to_string(res[team_index_map[0]]),
            [&res](const std::string &acc, size_t entry) -> std::string {
              return std::move(acc) + "," + std::to_string(res[entry]);
            })
     << "," << llh << "\n";
}

void write_csv_header(std::ostream                   &os,
                      const std::vector<std::string> &team_list) {
  os << std::accumulate(std::next(team_list.begin()),
                        team_list.end(),
                        *team_list.begin(),
                        [](const std::string &acc, const std::string &entry)
                            -> std::string { return acc + "," + entry; })
     << ",llh\n";
}

results_t &results_t::add_file_output(const std::filesystem::path &prefix) {
  _params_outfile = std::ofstream(params_filename(prefix));
  write_csv_header(*_params_outfile, _all_teams);

  _probs_outfile = std::ofstream(probs_filename(prefix));
  write_csv_header(*_probs_outfile, _bracket_teams);

  return *this;
}

results_t &results_t::enable_memory_save() {
  _result_list = std::vector<result_t>();
  return *this;
}

void results_t::write_params_line(const result_t &r) {
  write_csv_line(*_params_outfile, _all_team_index_map, r.params, r.llh);
}

void results_t::write_probs_line(const result_t &r) {
  write_csv_line(*_probs_outfile, _bracket_team_index_map, r.win_prob, r.llh);
}

void results_t::write_result_to_outfiles(const result_t &r) {
  write_params_line(r);
  write_probs_line(r);
}

void results_t::add_result(result_t &&r) {
  _sample_count += 1;

  if (_params_outfile.has_value() && _probs_outfile.has_value()) {
    write_result_to_outfiles(r);
  }

  if (_result_list.has_value()) { _result_list->push_back(r); }
}
