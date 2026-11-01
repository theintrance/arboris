include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Source files
set(ARBORIS_SOURCES
  dom/dom_manager.cc
  dom/dom_builder.cc
  dom/dom_indexer.cc
  dom/dom_subtree.cc
  dom/dom_query.cc
  dom/html_token_parser.cc
  encoding/encoding.cc
  selector/selector.cc
  string/decode.cc
  string/string_scalar.cc
  utils/tag.cc
  utils/tag_rules.cc
)

# Headers that <arboris/arboris.hpp> reaches, directly or through each other. These are
# what an install ships, so a program built against one compiles against exactly this set.
set(ARBORIS_PUBLIC_HEADERS
  dom/dom_indexer.hpp
  dom/dom_manager.hpp
  dom/dom_query.hpp
  dom/dom_store.hpp
  dom/dom_subtree.hpp
  dom/dom_types.hpp
  dom/node_ref.hpp
  dom/tag_node.hpp
  encoding/encoding.hpp
  selector/selector.hpp
  string/decode.hpp
  utils/assertion.hpp
  utils/html_tokens.hpp
  utils/query_options.hpp
  utils/string_arena.hpp
  utils/string_pool.hpp
  utils/tag.hpp
  utils/tokens.hpp
)

# The tokenizer and builder: used by the library's own sources only, never installed.
set(ARBORIS_PRIVATE_HEADERS
  dom/dom_builder.hpp
  dom/html_token_parser.hpp
  dom/token_parser.hpp
  string/named_references.inc
  string/string.hpp
  utils/name_set.hpp
  utils/set_utils.hpp
  utils/tag_rules.hpp
)

# Create arboris library. The namespaced name is the one an installed package exports, so a
# project that add_subdirectory()s or FetchContent()s arboris links the same arboris::arboris.
add_library(arboris ${ARBORIS_SOURCES} ${ARBORIS_PUBLIC_HEADERS} ${ARBORIS_PRIVATE_HEADERS})
add_library(arboris::arboris ALIAS arboris)

# The public header is <arboris/arboris.hpp>. The headers it includes refer to each other
# by their path under src/ ("dom/node_ref.hpp"), and are installed under include/arboris/
# with that layout, so an installed arboris puts both directories on the include path.
target_include_directories(arboris PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/arboris>
)

# Set compile features
target_compile_features(arboris PUBLIC cxx_std_20)

# ARBORIS_ASSERT checks the library's own invariants. Whether it compiles in follows the
# build type unless set with -DARBORIS_ASSERTIONS=ON|OFF, and is written into
# utils/config.hpp, which is installed with the other headers: inline code in a user's
# program then matches the library it links, whatever that program's own build type.
if (NOT DEFINED ARBORIS_ASSERTIONS)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ARBORIS_ASSERTIONS ON)
  else()
    set(ARBORIS_ASSERTIONS OFF)
  endif()
endif()
set(ARBORIS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
configure_file(utils/config.hpp.in ${ARBORIS_GENERATED_INCLUDE_DIR}/utils/config.hpp)
target_include_directories(arboris PUBLIC $<BUILD_INTERFACE:${ARBORIS_GENERATED_INCLUDE_DIR}>)

# Encodings other than UTF-8 and UTF-16 are converted through iconv. Without it, a page in
# one of them is read as UTF-8 and DOMManager::unsupported_encoding() says so.
option(ARBORIS_ICONV "Convert legacy encodings through iconv" ON)
if (ARBORIS_ICONV)
  find_package(Iconv)
  if (Iconv_FOUND)
    target_link_libraries(arboris PRIVATE Iconv::Iconv)
    target_compile_definitions(arboris PRIVATE ARBORIS_HAVE_ICONV)
    set(ARBORIS_HAVE_ICONV ON)
  else()
    message(STATUS "iconv not found; only UTF-8 and UTF-16 input will be converted")
  endif()
endif()

# Install: the library, the public headers under include/arboris/ in their src/ layout, and
# a package config so that `find_package(arboris)` gives a project the arboris::arboris target.
# Off by default when arboris is a subproject, so a parent's `cmake --install` does not get
# arboris's headers and package config in its prefix. Everything here is the "cpp" component;
# the Python wheel installs only its own.
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(ARBORIS_INSTALL_DEFAULT ON)
else()
  set(ARBORIS_INSTALL_DEFAULT OFF)
endif()
option(ARBORIS_INSTALL "Generate install rules for the library" ${ARBORIS_INSTALL_DEFAULT})
if (NOT ARBORIS_INSTALL)
  return()
endif()

install(TARGETS arboris
  EXPORT arborisTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT cpp
)
install(FILES ${PROJECT_SOURCE_DIR}/include/arboris/arboris.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/arboris
  COMPONENT cpp
)
foreach(header IN LISTS ARBORIS_PUBLIC_HEADERS)
  get_filename_component(header_dir ${header} DIRECTORY)
  install(FILES ${header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/arboris/${header_dir} COMPONENT cpp)
endforeach()
install(FILES ${ARBORIS_GENERATED_INCLUDE_DIR}/utils/config.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/arboris/utils
  COMPONENT cpp
)

set(ARBORIS_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/arboris)
install(EXPORT arborisTargets
  NAMESPACE arboris::
  DESTINATION ${ARBORIS_CMAKE_DIR}
  COMPONENT cpp
)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/arborisConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/arborisConfig.cmake
  INSTALL_DESTINATION ${ARBORIS_CMAKE_DIR}
)
# 0.x: the API may change from one minor version to the next, so find_package(arboris 0.1)
# must not accept 0.2.
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/arborisConfigVersion.cmake
  COMPATIBILITY SameMinorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/arborisConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/arborisConfigVersion.cmake
  DESTINATION ${ARBORIS_CMAKE_DIR}
  COMPONENT cpp
)
