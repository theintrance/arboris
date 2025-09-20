/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <iostream>
#include <memory>
#include <string_view>
#include <utility>

#include "dom/dom_builder.hpp"
#include "dom/html_tag_provider.hpp"

// TODO(team): move to test
int main() {
  // HTML content example
  std::string_view html_content =
      "<html><head><title>Test</title></head><body><h1>Hello World</h1><p>This is a test.</p></body></html>";

  // DOMBuilder creation
  auto dom_builder = std::make_shared<arboris::DOMBuilder>();

  // HtmlTagProvider creation
  arboris::HtmlTagProvider provider(html_content);

  // Connect callback
  provider.set_feed_open_token_callback([&dom_builder](arboris::HtmlToken&& token) -> bool {
    std::cout << "Open token received for tag: " << static_cast<int>(token.tag) << " (pos: " << token.begin_pos << "-"
              << token.end_pos << ")" << std::endl;
    bool result = dom_builder->FeedOpenToken(std::move(token));
    std::cout << "FeedOpenToken returned: " << (result ? "true" : "false") << std::endl;
    if (!result) {
      std::cout << "ERROR: FeedOpenToken failed!" << std::endl;
    }
    return result;
  });

  provider.set_feed_text_token_callback([&dom_builder](arboris::HtmlTextToken&& token) -> bool {
    std::cout << "Text token received: '" << token.text_content << "' (pos: " << token.begin_pos << "-" << token.end_pos
              << ")" << std::endl;
    bool result = dom_builder->FeedTextToken(std::move(token));
    std::cout << "FeedTextToken returned: " << (result ? "true" : "false") << std::endl;
    if (!result) {
      std::cout << "ERROR: FeedTextToken failed!" << std::endl;
    }
    return result;
  });

  provider.set_feed_close_token_callback([&dom_builder](arboris::HtmlCloseToken&& token) -> bool {
    std::cout << "Close token received for tag: " << static_cast<int>(token.tag) << " (pos: " << token.begin_pos << "-"
              << token.end_pos << ")" << std::endl;
    bool result = dom_builder->FeedCloseToken(std::move(token));
    std::cout << "FeedCloseToken returned: " << (result ? "true" : "false") << std::endl;
    if (!result) {
      std::cout << "ERROR: FeedCloseToken failed!" << std::endl;
    }
    return result;
  });

  // Execute HTML parsing
  if (provider.Parse()) {
    std::cout << "HTML parsing completed successfully!" << std::endl;
    std::cout << "DOM validation: " << (dom_builder->Validate() ? "Valid" : "Invalid") << std::endl;
  } else {
    std::cout << "HTML parsing failed!" << std::endl;
  }

  return 0;
}
