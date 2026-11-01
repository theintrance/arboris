include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Source files
set(ARBORIS_SOURCES
  arboris/dom/dom_manager.cc
  arboris/dom/dom_builder.cc
  arboris/dom/dom_indexer.cc
  arboris/dom/dom_subtree.cc
  arboris/dom/dom_query.cc
  arboris/dom/html_token_parser.cc
  arboris/encoding/encoding.cc
  arboris/selector/selector.cc
  arboris/string/decode.cc
  arboris/string/string_scalar.cc
  arboris/utils/tag.cc
  arboris/utils/tag_rules.cc
)

# Headers that <arboris/arboris.hpp> reaches, directly or through each other. These are
# what an install ships, so a program built against one compiles against exactly this set.
set(ARBORIS_PUBLIC_HEADERS
  arboris/dom/dom_indexer.hpp
  arboris/dom/dom_manager.hpp
  arboris/dom/dom_query.hpp
  arboris/dom/dom_store.hpp
  arboris/dom/dom_subtree.hpp
  arboris/dom/dom_types.hpp
  arboris/dom/node_ref.hpp
  arboris/dom/tag_node.hpp
  arboris/encoding/encoding.hpp
  arboris/selector/selector.hpp
  arboris/string/decode.hpp
  arboris/utils/assertion.hpp
  arboris/utils/html_tokens.hpp
  arboris/utils/query_options.hpp
  arboris/utils/string_arena.hpp
  arboris/utils/string_pool.hpp
  arboris/utils/tag.hpp
  arboris/utils/tokens.hpp
)

# The tokenizer and builder: used by the library's own sources only, never installed.
set(ARBORIS_PRIVATE_HEADERS
  arboris/dom/dom_builder.hpp
  arboris/dom/html_token_parser.hpp
  arboris/dom/token_parser.hpp
  arboris/string/named_references.inc
  arboris/string/string.hpp
  arboris/utils/name_set.hpp
  arboris/utils/set_utils.hpp
  arboris/utils/tag_rules.hpp
)

# Create arboris library. The namespaced name is the one an installed package exports, so a
# project that add_subdirectory()s or FetchContent()s arboris links the same arboris::arboris.
add_library(arboris ${ARBORIS_SOURCES} ${ARBORIS_PUBLIC_HEADERS} ${ARBORIS_PRIVATE_HEADERS})
add_library(arboris::arboris ALIAS arboris)

# Every header lives under an arboris/ directory and is included as "arboris/dom/node_ref.hpp",
# in the tree (src/arboris/, include/arboris/) and once installed (include/arboris/) alike.
# One include root, and no top-level name that could collide with a user's own headers.
target_include_directories(arboris PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Set compile features
target_compile_features(arboris PUBLIC cxx_std_20)

# ARBORIS_ASSERT checks the library's own invariants. Whether it compiles in follows the
# build type unless set with -DARBORIS_ASSERTIONS=ON|OFF, and is written into
# arboris/utils/config.hpp, which is installed with the other headers: inline code in a user's
# program then matches the library it links, whatever that program's own build type.
if (NOT DEFINED ARBORIS_ASSERTIONS)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ARBORIS_ASSERTIONS ON)
  else()
    set(ARBORIS_ASSERTIONS OFF)
  endif()
endif()
set(ARBORIS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
configure_file(arboris/utils/config.hpp.in ${ARBORIS_GENERATED_INCLUDE_DIR}/arboris/utils/config.hpp)
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

# Install: the library, the public headers under include/arboris/ as they sit in src/, and
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
  install(FILES ${header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${header_dir} COMPONENT cpp)
endforeach()
install(FILES ${ARBORIS_GENERATED_INCLUDE_DIR}/arboris/utils/config.hpp
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
