#ifndef SUMMARY_HPP
#define SUMMARY_HPP

#include <ostream>
#include <stdexcept>
#include <vector>
#include "tournament.hpp"
#include "dataset.hpp"

struct result_t {
  vector_t win_prob;
  params_t params;
  double lh;
};

std::ostream &operator<<(std::ostream &os, const result_t &r) {
  os << "{\"win_prob\": " << to_json(r.win_prob) << ", ";
  os << "\"params\": " << to_json(r.params) << ", ";
  os << "\"lh\": " << std::to_string(r.lh) << "}";
  return os;
}

class summary_t {
public:
  summary_t(const std::vector<result_t> &r) : _results{r} {}
  summary_t(std::vector<result_t> &&r) : _results{std::move(r)} {}

  void write(std::ostream &os, size_t burnin, size_t sample_iter) const;

private:
  std::vector<result_t> _results;
};

#endif
