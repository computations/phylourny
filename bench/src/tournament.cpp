#include <benchmark/benchmark.h>
#include <cstddef>
#include <tournament.hpp>

static void BM_tournament_factory(benchmark::State &state) {
  for(auto _ : state){
    benchmark::DoNotOptimize(tournament_factory(state.range(0)));
  }
}

BENCHMARK(BM_tournament_factory)->Range(1<<2, 1<<10);

static void BM_tourney_eval(benchmark::State &state){
  auto t = tournament_factory(state.range(0));
  auto m = uniform_matrix_factory(state.range(0));
  t.reset_win_probs(m);
  t.relabel_indicies();
  for(auto _: state){
    benchmark::DoNotOptimize(t.eval());
  }
}

BENCHMARK(BM_tourney_eval)->RangeMultiplier(2)->Range(1<<2, 1<<7);
