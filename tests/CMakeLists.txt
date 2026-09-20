function(add_gtest name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name}
    PRIVATE
    gtest
    gtest_main
    arboris
  )
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_gtest(example_test example_test.cc)
add_gtest(string_test string_test.cc)
add_gtest(html_token_parser_test html_token_parser_test.cc)
# TODO(team): add dom_manager_test.cc (missing from #24) and re-enable
# add_gtest(dom_manager_test dom_manager_test.cc)

# TODO(team): enable this test after fixing DomBuilder
# add_gtest(dom_builder_test dom_builder_test.cc)

# html5lib-tests tokenizer conformance (https://github.com/html5lib/html5lib-tests)
# Runs tests/html5lib via pytest (`uv` group "test"). Scope: every well-formed Data-state
# tokenizer case. Broken HTML is out of scope; see tests/html5lib/conftest.py.
option(ARBORIS_HTML5LIB_TESTS "Download html5lib-tests and register the tokenizer conformance test" OFF)

if (ARBORIS_HTML5LIB_TESTS)
  FetchContent_Declare(
    html5lib_tests
    URL https://github.com/html5lib/html5lib-tests/archive/224991ec10db04f056a89eed8b0bd8695fd2950e.zip
  )
  FetchContent_MakeAvailable(html5lib_tests)
  find_program(UV_EXECUTABLE uv REQUIRED)

  add_executable(html5lib_tokenizer_dump html5lib/tokenizer_dump.cc)
  target_link_libraries(html5lib_tokenizer_dump PRIVATE arboris)
  target_include_directories(html5lib_tokenizer_dump PRIVATE ${CMAKE_SOURCE_DIR}/src)

  add_test(NAME html5lib_tokenizer
    COMMAND ${UV_EXECUTABLE} run --group test pytest ${CMAKE_CURRENT_SOURCE_DIR}/html5lib -q
            --dump $<TARGET_FILE:html5lib_tokenizer_dump>
            --tests-dir ${html5lib_tests_SOURCE_DIR}/tokenizer
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
