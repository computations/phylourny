#include <benchmark/benchmark.h>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <tournament.hpp>

static void BM_tournament_factory(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(tournament_factory(state.range(0)));
  }
}

BENCHMARK(BM_tournament_factory)->Range(1 << 2, 1 << 10);

static void BM_tourney_eval(benchmark::State &state) {
  auto t = tournament_factory(state.range(0));
  auto m = uniform_matrix_factory(state.range(0));
  t.reset_win_probs(m);
  t.relabel_indicies();
  for (auto _ : state) {
    benchmark::DoNotOptimize(t.eval());
  }
}

BENCHMARK(BM_tourney_eval)->RangeMultiplier(2)->Range(1 << 2, 1 << 7);

constexpr inline double factorial1(uint64_t i) {
  if (i < factorial_table_size) {
    return factorial_table[i];
  }
  double f = factorial_table[factorial_table_size - 1];
  for (size_t k = factorial_table_size; k <= i; ++k) {
    f *= k;
  }
  return f;
}

constexpr inline double factorial2(uint64_t i) {
  if (i < factorial_table_size) {
    return factorial_table[i];
  }
  return factorial2(i - 1) * i;
}

constexpr inline uint64_t factorial3(uint64_t i) {
  if (i < factorial_table_size) {
    return factorial_table[i];
  }
  return factorial3(i - 1) * i;
}

constexpr inline uint64_t factorial4(uint64_t i) {
  if (i < factorial_table_size) {
    return factorial_table[i];
  }
  uint64_t f = factorial_table[factorial_table_size - 1];
  for (size_t k = factorial_table_size; k <= i; ++k) {
    f *= k;
  }
  return f;
}

static void BM_factorial1(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(factorial1(n));
  }
}

static void BM_factorial2(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(factorial2(n));
  }
}

static void BM_factorial3(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(factorial3(n));
  }
}

static void BM_factorial4(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(factorial4(n));
  }
}

BENCHMARK(BM_factorial1)->DenseRange(1, 21, 5);
BENCHMARK(BM_factorial2)->DenseRange(1, 21, 5);
BENCHMARK(BM_factorial3)->DenseRange(1, 21, 5);
BENCHMARK(BM_factorial4)->DenseRange(1, 21, 5);

constexpr inline double bestof_n1(double wp1, double wp2, uint64_t n) {
  uint64_t k = (n + 1) / 2;
  double sum = 0.0;
  for (size_t i = 0; i < k; ++i) {
    sum += pow(wp1, k) * pow(wp2, i) * combinations(k + i - 1, i);
  }
  return sum;
}

constexpr inline double bestof_n2(double wp1, double wp2, uint64_t n) {
  uint64_t k = (n + 1) / 2;
  double sum = 0.0;
  for (size_t i = 0; i < k; ++i) {
    double wpp1 = 1.0;
    for (size_t i = 0; i < k; ++i) {
      wpp1 *= wp1;
    }
    double wpp2 = 1.0;
    for (size_t i = 0; i < k; ++i) {
      wpp2 *= wp2;
    }
    sum += wpp1 * wpp2 * combinations(k + i - 1, i);
  }
  return sum;
}

constexpr inline double bestof_n3(double wp1, double wp2, uint64_t n) {
  uint64_t k = (n + 1) / 2;
  double sum = 0.0;
  for (size_t i = 0; i < k; ++i) {
    double wpp2 = 1.0;
    for (size_t i = 0; i < k; ++i) {
      wpp2 *= wp2;
    }
    sum += wpp2 * combinations(k + i - 1, i);
  }
  double wpp1 = 1.0;
  for (size_t i = 0; i < k; ++i) {
    wpp1 *= wp1;
  }
  return sum * wpp1;
}

constexpr inline double bestof_n4(double wp1, double wp2, uint64_t n) {
  uint64_t k = (n + 1) / 2;
  double sum = 0.0;
  for (size_t i = 0; i < k; ++i) {
    sum += int_pow(wp2, i) * combinations(k + i - 1, i);
  }
  return sum * int_pow(wp1, k);
}

static void BM_bestof_n1(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(bestof_n1(0.5, 0.5, n));
  }
}

static void BM_bestof_n2(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(bestof_n2(0.5, 0.5, n));
  }
}

static void BM_bestof_n3(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(bestof_n3(0.5, 0.5, n));
  }
}

static void BM_bestof_n4(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(bestof_n4(0.5, 0.5, n));
  }
}

BENCHMARK(BM_bestof_n1)->DenseRange(1, 11, 2);
BENCHMARK(BM_bestof_n2)->DenseRange(1, 11, 2);
BENCHMARK(BM_bestof_n3)->DenseRange(1, 11, 2);
BENCHMARK(BM_bestof_n4)->DenseRange(1, 11, 2);

constexpr inline double int_pow1(double base, uint64_t k) {
  if (k == 0) {
    return 1.0;
  }
  double wpp1 = base;
  for (size_t i = 1; i < k; ++i) {
    wpp1 *= base;
  }
  return wpp1;
}

constexpr inline double int_pow2(double base, uint64_t k) {
  double wpp1 = 1.0;
  for (size_t i = 0; i < k; ++i) {
    wpp1 *= base;
  }
  return wpp1;
}

static void BM_int_pow1(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(int_pow1(0.5, n));
  }
}

static void BM_int_pow2(benchmark::State &state) {
  uint64_t n = state.range(0);
  for (auto _ : state) {
    benchmark::DoNotOptimize(int_pow2(0.5, n));
  }
}

BENCHMARK(BM_int_pow1)->DenseRange(1, 11, 2);
BENCHMARK(BM_int_pow2)->DenseRange(1, 11, 2);
