#include "summary.hpp"

void summary_t::write(std::ostream &os, size_t burnin,
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
