#ifndef SUMMARY_HPP
#define SUMMARY_HPP

#include "mcmc.hpp"
#include "util.hpp"
#include <optional>
#include <ostream>
#include <vector>

struct result_t {
  vector_t                win_prob;
  params_t                params;
  std::optional<matrix_t> prob_matrix;
  double                  llh;
};

auto operator<<(std::ostream &os, const result_t &r) -> std::ostream &;

class summary_t {
public:
  explicit summary_t(const std::vector<result_t> &r) : _results{r} {}
  explicit summary_t(std::vector<result_t> &&r) : _results{std::move(r)} {}

  void write_samples(std::ostream &os, size_t burnin, size_t sample_iter) const;
  void write_samples_csv_win_probs(std::ostream                   &os,
                                   const std::vector<std::string> &team_list,
                                   const team_name_map_t          &name_map,
                                   size_t                          burnin,
                                   size_t sample_iter) const;
  void write_samples_csv_params(std::ostream          &os,
                                const team_name_map_t &name_map,
                                size_t                 burnin,
                                size_t                 sample_iter) const;
  void write_mlp(std::ostream &os, size_t burnin) const;
  void write_mmpp(std::ostream &os, size_t burnin) const;

private:
  [[nodiscard]] auto compute_mlp(size_t burnin) const -> vector_t;
  [[nodiscard]] auto compute_mmpp(size_t burnin) const -> vector_t;

  std::vector<result_t> _results;
};

#endif
