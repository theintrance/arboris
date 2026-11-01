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

# Header files
set(ARBORIS_HEADERS
  dom/dom_manager.hpp
  dom/dom_builder.hpp
  dom/dom_subtree.hpp
  dom/dom_query.hpp
  dom/token_parser.hpp
  dom/html_token_parser.hpp
  dom/dom_store.hpp
  dom/node_ref.hpp
  dom/dom_indexer.hpp
  dom/dom_types.hpp
  dom/tag_node.hpp
  encoding/encoding.hpp
  selector/selector.hpp
  string/decode.hpp
  string/named_references.inc
  string/string.hpp
  utils/assertion.hpp
  utils/html_tokens.hpp
  utils/name_set.hpp
  utils/query_options.hpp
  utils/set_utils.hpp
  utils/string_arena.hpp
  utils/string_pool.hpp
  utils/tag.hpp
  utils/tag_rules.hpp
  utils/tokens.hpp
)

# Create arboris library
add_library(arboris ${ARBORIS_SOURCES} ${ARBORIS_HEADERS})

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

# ARBORIS_ASSERT checks the library's own invariants and compiles out unless ARBORIS_DEBUG
# is defined. Debug builds, and so the Debug and sanitizer CI jobs, run with it on.
target_compile_definitions(arboris PUBLIC $<$<CONFIG:Debug>:ARBORIS_DEBUG>)

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

# Install: the library, every header under include/arboris/, and a package config so that
# `find_package(arboris)` gives a project the arboris::arboris target.
install(TARGETS arboris
  EXPORT arborisTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES ${PROJECT_SOURCE_DIR}/include/arboris/arboris.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/arboris
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/arboris
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inc"
)

set(ARBORIS_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/arboris)
install(EXPORT arborisTargets
  NAMESPACE arboris::
  DESTINATION ${ARBORIS_CMAKE_DIR}
)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/arborisConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/arborisConfig.cmake
  INSTALL_DESTINATION ${ARBORIS_CMAKE_DIR}
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/arborisConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/arborisConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/arborisConfigVersion.cmake
  DESTINATION ${ARBORIS_CMAKE_DIR}
)
