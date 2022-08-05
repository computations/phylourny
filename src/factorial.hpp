#ifndef FACTORIAL_HPP
#define FACTORIAL_HPP

#include "debug.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

constexpr size_t factorial_table_size = 11;

constexpr std::array<double, factorial_table_size> factorial_table = {
    1,
    1,
    2,
    6,
    24,
    120,
    720,
    5040,
    40320,
    362880,
    3628800,
};

/**
 * Compute a factorial. Uses a lookup table to accelerate computation.
 */
constexpr inline auto factorial(uint64_t i) -> double {
  if (i > 170) { return INFINITY; }
  if (i < factorial_table_size) { return factorial_table.at(i); }
  double f = factorial_table[factorial_table_size - 1];
  for (size_t k = factorial_table_size; k <= i; ++k) {
    f *= static_cast<double>(k);
  }
  return f;
}

constexpr inline auto log_factorial(size_t i) -> double {
  if (i < factorial_table_size) { return std::log(factorial_table.at(i)); }
  double f = std::log(factorial_table[factorial_table_size - 1]);
  for (size_t k = factorial_table_size; k <= i; ++k) {
    f += std::log(static_cast<double>(k));
  }
  return f;
}

/**
 * Computes C(n,i) using the accelerated `factorial` function
 */
constexpr inline auto combinations(uint64_t n, uint64_t i) -> double {
  return factorial(n) / (factorial(i) * factorial(n - i));
}

/**
 * A fast version of pow for integer exponents.
 */
constexpr inline auto int_pow(double base, uint64_t k) -> double {
  if (k == 0) { return 1.0; }
  double wpp1 = base;
  for (size_t i = 1; i < k; ++i) { wpp1 *= base; }
  return wpp1;
}

constexpr inline auto bestof_n(double wp1, double wp2, uint64_t n) -> double {
  assert_string(wp1 <= 1.0, "Win prob is not well formed");
  assert_string(wp2 <= 1.0, "Win prob is not well formed");

  assert_string(wp1 >= 0.0, "Win prob is not well formed");
  assert_string(wp2 >= 0.0, "Win prob is not well formed");

  assert_string(fabs(wp1 + wp2 - 1.0) < 1e-14, "Win prob is not well formed");

  uint64_t k    = (n + 1) / 2;
  double   sum  = 0.0;
  double   wpp1 = int_pow(wp1, k);
  for (size_t i = 0; i < k; ++i) {
    sum += int_pow(wp2, i) * combinations(k + i - 1, i);
  }
  return sum * wpp1;
}

#endif
