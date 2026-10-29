# Source files
set(ARBORIS_SOURCES
  dom/dom_manager.cc
  dom/dom_builder.cc
  dom/dom_indexer.cc
  dom/dom_subtree.cc
  dom/dom_query.cc
  dom/html_token_parser.cc
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
  dom/dom_query.hpp
  dom/dom_subtree.hpp
  dom/dom_indexer.hpp
  dom/dom_types.hpp
  dom/tag_node.hpp
  string/decode.hpp
  string/named_references.inc
  string/string.hpp
  utils/html_tokens.hpp
  utils/name_set.hpp
  utils/string_arena.hpp
  utils/tag.hpp
  utils/tag_rules.hpp
  utils/tokens.hpp
)

# Create arboris library
add_library(arboris ${ARBORIS_SOURCES} ${ARBORIS_HEADERS})

# Set include directories
target_include_directories(arboris PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# Set compile features
target_compile_features(arboris PUBLIC cxx_std_20)

# Install targets
install(TARGETS arboris
  EXPORT arborisTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Install headers
install(FILES ${ARBORIS_HEADERS}
  DESTINATION include/arboris
)
