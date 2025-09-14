# Source files
set(ARBORIS_SOURCES
  dom/dom_container.cc
  parser/html_parser.cc
  string/string_scalar.cc
  utils/tag.cc
)

# Header files
set(ARBORIS_HEADERS
  dom/dom_container.hpp
  dom/node.hpp
  parser/html_parser.hpp
  string/string.hpp
  utils/html_tokens.hpp
  utils/tag.hpp
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
