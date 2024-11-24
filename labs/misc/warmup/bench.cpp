#include "benchmark/benchmark.h"
#include "solution.h"
#include <iostream>

template <typename Solver>
static void bench(benchmark::State &state, Solver solver)
{
  // problem: count sum of all the numbers up to N
  constexpr int N = 1000;
  int arr[N];
  for (int i = 0; i < N; i++) {
    arr[i] = i + 1;
  }

  int result = 0;

  // benchmark
  for (auto _ : state) {
    result = solver.solution(arr, N);
    benchmark::DoNotOptimize(arr);
  }
}

static void baseline(benchmark::State &state)
{
  bench(state, Baseline{});
}

static void baseline128(benchmark::State &state)
{
  bench(state, Baseline128{});
}

static void baseline256(benchmark::State &state)
{
  bench(state, Baseline256{});
}

static void baseline512(benchmark::State &state)
{
  bench(state, Baseline512{});
}

static void formula(benchmark::State &state)
{
  bench(state, Formula{});
}

// Register the function as a benchmark
BENCHMARK(baseline);
BENCHMARK(baseline128);
BENCHMARK(baseline256);
BENCHMARK(baseline512);
BENCHMARK(formula);

// Run the benchmark
BENCHMARK_MAIN();
