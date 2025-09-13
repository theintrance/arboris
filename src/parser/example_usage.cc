/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "parser/html_parser.hpp"

#include <iostream>
#include <string>

int main() {
  // Simple HTML example
  std::string html = R"(
    <html>
      <head>
        <title>Test Page</title>
      </head>
      <body>
        <h1>Hello World</h1>
        <p>This is a test paragraph.</p>
        <div>
          <span>Nested content</span>
        </div>
      </body>
    </html>
  )";

  // Create HTMLParser and parse
  arboris::HTMLParser parser(html);
  auto dom_container = parser.parse();

  // Validate parsing result
  if (dom_container->validate()) {
    std::cout << "HTML parsing completed successfully!" << std::endl;
  } else {
    std::cout << "An error occurred during HTML parsing." << std::endl;
  }

  return 0;
}
