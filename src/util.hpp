#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>

typedef std::vector<std::vector<double>> matrix_t;
typedef std::vector<double> vector_t;

matrix_t uniform_matrix_factory(size_t n);
matrix_t random_matrix_factory(size_t n, uint64_t seed);

double compute_entropy(const vector_t &v);

double compute_perplexity(const vector_t &v);

std::string to_json(const matrix_t &m);
std::string to_json(const vector_t &m);
std::string to_json(const std::vector<size_t> &m);

std::string to_string(const matrix_t &m);
std::string to_string(const vector_t &m);
std::string to_string(const std::vector<size_t> &m);

#endif // UTIL_HPP
