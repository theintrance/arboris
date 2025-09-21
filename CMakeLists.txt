cmake_minimum_required(VERSION 3.14)
project(arboris)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include Modules
include(FetchContent)
include(CTest)

# Download dependencies
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
if (WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Add subdirectories
add_subdirectory(src)

if (NOT NO_TESTS)
  add_subdirectory(tests)
endif()

if (NOT NO_BENCHMARK)
  add_subdirectory(benchmark)
endif()

# TODO(team): move to test
add_executable(example_usage example_usage.cc)
target_link_libraries(example_usage arboris)
target_include_directories(example_usage PRIVATE src)
