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
add_gtest(html_tag_provider_test html_tag_provider_test.cc)

# TODO(team): enable this test after DOMBuilder is implemented
# add_gtest(dom_builder_test dom_builder_test.cc)
