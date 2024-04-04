#include "summary.hpp"
#include "mcmc.hpp"
#include "tournament.hpp"
#include "util.hpp"
#include <cmath>
#include <cstddef>
#include <limits>

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

/**
 * Write samples in a JSON format to a stream. The output format for this
 * function is JSON.
 *
 * @param os Output stream to insert the samples. The output format is JSON.
 *
 * @param burnin Number of initial samples to discard before summarizing.
 *
 * @param sample_iter Sample ever `sample_iter` results after the burnin.
 */
void summary_t::write_samples(std::ostream &os,
                              size_t        burnin,
                              size_t        sample_iter) const {
  if (burnin > _results.size()) {
    throw std::runtime_error("Burnin is longer than results");
  }
  os << "[\n";
  for (size_t i = burnin; i < _results.size(); i += sample_iter) {
    os << _results[i] << ",\n";
  }
  os.seekp(-2, std::ostream::cur);
  os << "]\n";
}

template <typename T>
void write_csv_line(std::ostream              &os,
                    const std::vector<size_t> &team_index_map,
                    const T                   &res) {
  os << std::accumulate(
            std::next(team_index_map.begin()),
            team_index_map.end(),
            std::to_string(res[team_index_map[0]]),
            [&res](const std::string &acc, size_t entry) -> std::string {
              return std::move(acc) + "," + std::to_string(res[entry]);
            })
     << "\n";
}

void write_csv_header_win_prob(std::ostream                   &os,
                               const std::vector<std::string> &team_list) {
  os << std::accumulate(std::next(team_list.begin()),
                        team_list.end(),
                        *team_list.begin(),
                        [](const std::string &acc, const std::string &entry)
                            -> std::string { return acc + "," + entry; })
     << "\n";
}

void summary_t::write_samples_csv_win_probs(
    std::ostream                   &os,
    const std::vector<std::string> &team_list,
    const team_name_map_t          &name_map,
    size_t                          burnin,
    size_t                          sample_iter) const {

  write_csv_header_win_prob(os, team_list);

  std::vector<size_t> team_index_map;
  team_index_map.resize(team_list.size());

  for (size_t i = 0; i < team_list.size(); ++i) {
    team_index_map[i] = name_map.at(team_list[i]);
  }

  for (size_t i = burnin; i < _results.size(); i += sample_iter) {
    write_csv_line(os, team_index_map, _results[i].win_prob);
  }
}

void summary_t::write_samples_csv_params(std::ostream          &os,
                                         const team_name_map_t &name_map,
                                         size_t                 burnin,
                                         size_t sample_iter) const {

  std::vector<std::string> team_list;
  team_list.reserve(name_map.size());
  for (const auto &kv : name_map) { team_list.push_back(kv.first); }

  write_csv_header_win_prob(os, team_list);

  std::vector<size_t> team_index_map;
  team_index_map.resize(team_list.size());

  for (size_t i = 0; i < team_list.size(); ++i) {
    team_index_map[i] = name_map.at(team_list[i]);
  }

  for (size_t i = burnin; i < _results.size(); i += sample_iter) {
    write_csv_line(os, team_index_map, _results[i].params);
  }
}

/**
 * Compute and write the maximum likelihood prediction to a stream
 *
 * @param os Stream to output results. Results are written in JSON format.
 *
 * @param burnin Initial samples to discard before computing the MLP.
 */
void summary_t::write_mlp(std::ostream &os, size_t burnin) const {
  auto mpp = compute_mlp(burnin);
  os << to_json(mpp) << std::endl;
}

/**
 * Compute and write the maximum marginal posterior prediction to the stream.
 *
 * @param os Stream to output results. Results are written in JSON format.
 *
 * @param burnin Initial samples to discard before compute the MMPP.
 */
void summary_t::write_mmpp(std::ostream &os, size_t burnin) const {
  auto mmpp = compute_mmpp(burnin);
  os << to_json(mmpp) << std::endl;
}

auto summary_t::compute_mlp(size_t burnin) const -> vector_t {
  if (burnin > _results.size()) {
    throw std::runtime_error("Burnin is longer than result for mlps");
  }

  vector_t best_probs = _results[0].win_prob;
  double   best_llh   = -std::numeric_limits<double>::infinity();

  for (size_t i = burnin; i < _results.size(); ++i) {
    if (best_llh < _results[i].llh) {
      best_llh   = _results[i].llh;
      best_probs = _results[i].win_prob;
    }
  }
  return best_probs;
}

auto summary_t::compute_mmpp(size_t burnin) const -> vector_t {
  if (burnin > _results.size()) {
    throw std::runtime_error("Burnin is longer than results for mmpp");
  }

  vector_t avg_probs(_results.front().win_prob.size(), 0);

  size_t total_iters = 0;
  for (size_t i = burnin; i < _results.size(); ++i) {
    total_iters++;
    for (size_t j = 0; j < avg_probs.size(); j++) {
      avg_probs[j] += _results[i].win_prob[j];
    }
  }

  for (double &avg_prob : avg_probs) {
    avg_prob /= static_cast<double>(total_iters);
  }

  return avg_probs;
}
