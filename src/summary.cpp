#include "summary.hpp"
#include "tournament.hpp"
#include <cmath>
#include <limits>

std::ostream &operator<<(std::ostream &os, const result_t &r) {
  os << "{\"win_prob\": " << to_json(r.win_prob) << ", ";
  os << "\"params\": " << to_json(r.params) << ", ";
  os << "\"llh\": " << std::to_string(r.llh) << "}";
  return os;
}

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

void summary_t::write_mpp(std::ostream &os) const {
  auto mpp = compute_mpp();
  os << to_json(mpp) << std::endl;
}

void summary_t::write_mmpp(std::ostream &os) const {
  auto mmpp = compute_mmpp();
  os << to_json(mmpp) << std::endl;
}

vector_t summary_t::compute_mpp() const {
  vector_t best_probs = _results[0].win_prob;
  double best_llh = -std::numeric_limits<double>::infinity();
  for (auto &r : _results) {
    if (best_llh < r.llh) {
      best_llh = r.llh;
      best_probs = r.win_prob;
    }
  }
  return best_probs;
}

vector_t summary_t::compute_mmpp() const {
  vector_t avg_probs(_results.front().win_prob.size(), 0);

  double max_llh = -std::numeric_limits<double>::infinity();
  for (auto &r : _results) {
    max_llh = std::max(r.llh, max_llh);
  }

  double total_lh = 0;
  for (auto &r : _results) {
    total_lh += std::exp(r.llh - max_llh);
  }

  for (auto &r : _results) {
    double weight = exp(r.llh - max_llh) / total_lh;
    for (size_t i = 0; i < avg_probs.size(); i++) {
      avg_probs[i] += r.win_prob[i] * weight;
    }
  }
  return avg_probs;
}
