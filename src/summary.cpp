#include "summary.hpp"
#include "tournament.hpp"
#include <cmath>
#include <cstddef>
#include <limits>

std::ostream &operator<<(std::ostream &os, const result_t &r) {
  os << "{\"win_prob\": " << to_json(r.win_prob) << ", ";
  os << "\"params\": " << to_json(r.params) << ", ";
  os << "\"llh\": " << std::to_string(r.llh) << "}";
  return os;
}

/**
 * Write samples in a CSV format to a stream. The output format for this
 * function is JSON.
 *
 * @param os Output stream to insert the samples. The output format is JSON.
 *
 * @param burnin Number of initial samples to discard before summarizing.
 *
 * @param sample_iter Sample ever `sample_iter` results after the burnin.
 */
void summary_t::write_samples(std::ostream &os, size_t burnin,
                              size_t sample_iter) const {
  if (burnin > _results.size()) {
    throw std::runtime_error("Burnin is longer than results");
  }
  os << "[\n";
  for (size_t i = burnin; i < _results.size(); i += sample_iter) {
    os << _results[i] << ",\n";
  }
  os.seekp(-2, os.cur);
  os << "]\n";
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

vector_t summary_t::compute_mlp(size_t burnin) const {
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

vector_t summary_t::compute_mmpp(size_t burnin) const {
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

  for (size_t i = 0; i < avg_probs.size(); i++) {
    avg_probs[i] /= static_cast<double>(total_iters);
  }

  return avg_probs;
}
