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
add_gtest(dom_manager_test dom_manager_test.cc)

# TODO(team): enable this test after fixing DomBuilder
# add_gtest(dom_builder_test dom_builder_test.cc)
