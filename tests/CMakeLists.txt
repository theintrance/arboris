function(add_gtest name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name}
    PRIVATE
      gtest
      gtest_main
  )
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_gtest(example_test example_test.cc)