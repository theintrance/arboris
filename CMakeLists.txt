cmake_minimum_required(VERSION 3.14)
project(arboris)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include Modules
include(FetchContent)
include(CTest)

# Set up compile commands JSON for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Download dependencies
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
if (WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

# html5lib-tests (https://github.com/html5lib/html5lib-tests), used by the tokenizer
# conformance test and the benchmark. Declared here so both use the same commit.
FetchContent_Declare(
  html5lib_tests
  URL https://github.com/html5lib/html5lib-tests/archive/224991ec10db04f056a89eed8b0bd8695fd2950e.zip
)

# Add subdirectories
add_subdirectory(src)

if (NOT NO_TESTS)
  add_subdirectory(tests)
endif()

if (NOT NO_BENCHMARK)
  add_subdirectory(benchmark)
endif()
