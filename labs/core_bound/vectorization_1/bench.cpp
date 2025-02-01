#include "benchmark/benchmark.h"
#include "solution.hpp"

template <typename Solver>
static void compute_alignment(benchmark::State &state, Solver solver) {
  auto [sequences1, sequences2] = init();

  for (auto _ : state) {
    Solver solver;
    auto res = solver.compute_alignment(sequences1, sequences2);
    benchmark::DoNotOptimize(res);
  }
}

static void baseline(benchmark::State &state)
{
  compute_alignment(state, Baseline{});
}

static void solution(benchmark::State &state)
{
  compute_alignment(state, Solution{});
}

static void video(benchmark::State &state)
{
  compute_alignment(state, VideoSolution{});
}

// Register the functions as a benchmark
BENCHMARK(baseline);
BENCHMARK(solution);
BENCHMARK(video);

// Run the benchmark
BENCHMARK_MAIN();
