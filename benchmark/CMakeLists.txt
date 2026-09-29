FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark html5lib_tests)

find_program(UV_EXECUTABLE uv REQUIRED)

option(ARBORIS_BENCH_LEXBOR "Also benchmark the lexbor tokenizer (https://github.com/lexbor/lexbor)" OFF)

set(ARBORIS_BENCH_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)

add_executable(arboris_bench bench_main.cc arboris_tokenizer.cc)
target_link_libraries(arboris_bench PRIVATE arboris benchmark::benchmark)
target_include_directories(arboris_bench PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(arboris_bench PRIVATE ARBORIS_BENCH_DATA_DIR="${ARBORIS_BENCH_DATA_DIR}")

if (ARBORIS_BENCH_LEXBOR)
  FetchContent_Declare(
    lexbor
    URL https://github.com/lexbor/lexbor/archive/refs/tags/v3.0.0.zip
    URL_HASH SHA256=7a791c914cadc0ac24ae0c2421d53ea8e92d840a99f4e01ee1519b8d243eff68
  )
  set(LEXBOR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
  set(LEXBOR_BUILD_STATIC ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(lexbor)

  target_sources(arboris_bench PRIVATE lexbor_tokenizer.cc)
  target_link_libraries(arboris_bench PRIVATE lexbor_static)
  target_compile_definitions(arboris_bench PRIVATE ARBORIS_BENCH_LEXBOR)
endif()

# Marks which html5lib-tests cases arboris tokenizes correctly. Shared with tests/ when
# ARBORIS_HTML5LIB_TESTS is on.
if (NOT TARGET html5lib_tokenizer_dump)
  add_executable(html5lib_tokenizer_dump ${PROJECT_SOURCE_DIR}/tests/html5lib/tokenizer_dump.cc)
  target_link_libraries(html5lib_tokenizer_dump PRIVATE arboris)
endif()

set(HTML5LIB_DIR ${PROJECT_SOURCE_DIR}/tests/html5lib)
add_custom_command(
  OUTPUT ${ARBORIS_BENCH_DATA_DIR}/html5lib_cases
  COMMAND ${UV_EXECUTABLE} run --group test python ${CMAKE_CURRENT_SOURCE_DIR}/export_cases.py
          --tests-dir ${html5lib_tests_SOURCE_DIR}/tokenizer
          --dump $<TARGET_FILE:html5lib_tokenizer_dump>
          --output ${ARBORIS_BENCH_DATA_DIR}/html5lib_cases
  DEPENDS export_cases.py html5lib_tokenizer_dump
          ${HTML5LIB_DIR}/conftest.py ${HTML5LIB_DIR}/categories.py ${HTML5LIB_DIR}/tokenizer_excludes.txt
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  VERBATIM)

add_custom_target(arboris_bench_data
  COMMAND ${CMAKE_COMMAND} -DDEST=${ARBORIS_BENCH_DATA_DIR}/corpus -P ${CMAKE_CURRENT_SOURCE_DIR}/corpus.cmake
  DEPENDS ${ARBORIS_BENCH_DATA_DIR}/html5lib_cases
  VERBATIM)
add_dependencies(arboris_bench arboris_bench_data)
