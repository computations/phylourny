#ifndef DATASET_HPP
#define DATASET_HPP

#include "match.hpp"
#include "util.hpp"
#include <vector>

class likelihood_model_t {
public:
  [[nodiscard]] virtual auto likelihood(const params_t &) const -> double = 0;

  [[nodiscard]] virtual auto log_likelihood(const params_t &p) const -> double {
    return std::log(likelihood(p));
  }

  [[nodiscard]] virtual auto param_count() const -> size_t = 0;

  virtual ~likelihood_model_t() = default;

  [[nodiscard]] virtual auto
  generate_win_probs(const params_t            &params,
                     const std::vector<size_t> &team_indicies) const
      -> matrix_t = 0;
};

/**
 * A `simple_likelihood_model_t` is a class used to compute the likelihood of
 * plausible pairwise win matrices. The major concepts are the match list, and
 * the likeliehoood function.
 */
class simple_likelihood_model_t final : public likelihood_model_t {
public:
  explicit simple_likelihood_model_t(const std::vector<match_t> &matches);
  ~simple_likelihood_model_t() override = default;

  [[nodiscard]] auto likelihood(const params_t &team_strs) const
      -> double override {
    return std::exp(log_likelihood(team_strs));
  }

  [[nodiscard]] auto log_likelihood(const params_t &team_win_probs) const
      -> double override;

  [[nodiscard]] auto param_count() const -> size_t override {
    return (_param_count * (_param_count + 1)) / 2;
  }

  [[nodiscard]] auto
  generate_win_probs(const params_t            &params,
                     const std::vector<size_t> &team_indicies) const
      -> matrix_t override;

private:
  std::vector<std::vector<unsigned int>> _win_matrix;
  size_t                                 _param_count;
};

class poisson_likelihood_model_t final : public likelihood_model_t {
public:
  explicit poisson_likelihood_model_t(const std::vector<match_t> &matches) :
      _param_count{count_teams(matches) + 1}, _matches{matches} {}
  ~poisson_likelihood_model_t() override = default;

  [[nodiscard]] auto likelihood(const params_t &team_strs) const
      -> double override {
    return std::exp(log_likelihood(team_strs));
  }

  [[nodiscard]] auto log_likelihood(const params_t &team_strs) const
      -> double override;

  [[nodiscard]] auto param_count() const -> size_t override {
    return _param_count;
  }

  [[nodiscard]] auto
  generate_win_probs(const params_t            &params,
                     const std::vector<size_t> &team_indicies) const
      -> matrix_t override;

private:
  size_t               _param_count;
  std::vector<match_t> _matches;
};
#endif
