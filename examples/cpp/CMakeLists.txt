add_executable(parse_and_query parse_and_query.cc)

target_include_directories(parse_and_query PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(parse_and_query PRIVATE arboris)
