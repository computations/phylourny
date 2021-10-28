#ifndef SUMMARY_HPP
#define SUMMARY_HPP

#include "dataset.hpp"
#include "tournament.hpp"
#include <ostream>
#include <stdexcept>
#include <vector>

struct result_t {
  vector_t win_prob;
  params_t params;
  double   llh;
};

std::ostream &operator<<(std::ostream &os, const result_t &r);

class summary_t {
public:
  explicit summary_t(const std::vector<result_t> &r) : _results{r} {}
  explicit summary_t(std::vector<result_t> &&r) : _results{std::move(r)} {}

  void write_samples(std::ostream &os, size_t burnin, size_t sample_iter) const;
  void write_mlp(std::ostream &os, size_t burnin) const;
  void write_mmpp(std::ostream &os, size_t burnin) const;

private:
  vector_t compute_mlp(size_t burnin) const;
  vector_t compute_mmpp(size_t burnin) const;

  std::vector<result_t> _results;
};

#endif
