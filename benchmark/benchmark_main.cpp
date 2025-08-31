/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <benchmark/benchmark.h>

void BM_Example(benchmark::State& state) {  // NOLINT(runtime/references)
  for (auto _ : state) {
    // Benchmark code
  }
}

BENCHMARK(BM_Example);

BENCHMARK_MAIN();
