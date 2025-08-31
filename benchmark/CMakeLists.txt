add_executable(arboris_bench benchmark_main.cpp)

target_link_libraries(arboris_bench
  PRIVATE
    benchmark::benchmark
)
