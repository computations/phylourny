#ifndef UTIL_HPP
#define UTIL_HPP

#include <random>
#include <string>
#include <sul/dynamic_bitset.hpp>
#include <vector>

typedef std::vector<std::vector<double>> matrix_t;
typedef std::vector<double>              vector_t;
typedef sul::dynamic_bitset<>            tip_bitset_t;
typedef std::mt19937_64                  random_engine_t;
typedef size_t                           clock_tick_t;

/**
 * TEST INFO FROM HEADER
 */
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

vector_t softmax(const vector_t &v);

std::string compute_base26(size_t i);

#endif // UTIL_HPP
