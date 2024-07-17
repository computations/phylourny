#include "results.hpp"

#include <fstream>
#include <iterator>
#include <string>

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

template <typename Iterator>
inline std::string make_csv_row(Iterator begin, Iterator end) {
  using value_type = typename std::iterator_traits<Iterator>::value_type;
  return std::accumulate(
             std::next(begin),
             end,
             std::string(*begin),
             [](std::string acc, const value_type &entry) -> std::string {
               acc += ", ";
               acc += entry;
               return acc;
             }) +
         "\n";
}

results_t &results_t::add_file_output(const std::filesystem::path &prefix) {
  _params_outfile = std::ofstream(params_filename(prefix));
  auto tmp        = _all_teams;
  tmp.push_back("llh");
  *_params_outfile << make_csv_row(tmp.begin(), tmp.end());

  _probs_outfile = std::ofstream(probs_filename(prefix));
  tmp            = _bracket_teams;
  tmp.push_back("llh");
  *_probs_outfile << make_csv_row(tmp.begin(), tmp.end());

  return *this;
}

results_t &results_t::enable_memory_save() {
  _result_list = std::vector<result_t>();
  return *this;
}

results_t &results_t::add_node_probs_output(const std::filesystem::path &prefix) {
  _node_probs_outfile = std::ofstream(node_probs_filename(prefix));

  std::vector<std::string> tmp{"node"};
  for (const auto &n : _bracket_teams) { tmp.push_back(n); }
  tmp.push_back("llh");
  *_node_probs_outfile << make_csv_row(tmp.begin(), tmp.end());

  return *this;
}

template <typename T>
std::vector<std::string>
make_temporary_vector(const std::vector<T>      vals,
                      const std::vector<size_t> index_map,
                      double                    llh) {
  std::vector<std::string> tmp;
  tmp.reserve(index_map.size() + 1);
  for (auto index : index_map) { tmp.push_back(std::to_string(vals[index])); }
  tmp.push_back(std::to_string(llh));
  return tmp;
}

void results_t::write_params_line(const result_t &r) {
  auto tmp = make_temporary_vector(r.params, _all_team_index_map, r.llh);
  *_params_outfile << make_csv_row(tmp.begin(), tmp.end());
}

void results_t::write_probs_line(const result_t &r) {
  auto tmp = make_temporary_vector(r.win_prob, _bracket_team_index_map, r.llh);
  *_probs_outfile << make_csv_row(tmp.begin(), tmp.end());
}

void results_t::write_node_probs_line(const result_t &r) {
  for (auto &kv : *r.node_probs) {
    std::vector<std::string> tmp;
    tmp.push_back(kv.first);
    for (auto f : kv.second) { tmp.push_back(std::to_string(f)); }
    tmp.push_back(std::to_string(r.llh));

    *_node_probs_outfile << make_csv_row(tmp.begin(), tmp.end());
  }
}

void results_t::write_result_to_outfiles(const result_t &r) {
  write_params_line(r);
  write_probs_line(r);
  if (_node_probs_outfile.has_value()) { write_node_probs_line(r); }
}

void results_t::add_result(result_t &&r) {
  _sample_count += 1;

  if (_params_outfile.has_value() && _probs_outfile.has_value()) {
    write_result_to_outfiles(r);
  }

  if (_result_list.has_value()) { _result_list->push_back(r); }
}
