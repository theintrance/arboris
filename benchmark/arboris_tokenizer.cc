/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

// The only file that calls into arboris, so an internal API change is fixed here alone.

#include <memory>
#include <string_view>

#include "benchmark/tokenizers.hpp"
#include "dom/html_token_parser.hpp"
#include "utils/html_tokens.hpp"
#include "utils/string_pool.hpp"

namespace arboris::bench {

TokenizeResult TokenizeArboris(std::string_view input) {
  TokenizeResult result;
  auto string_pool = std::make_shared<StringPool>(input.size());
  HtmlTokenParser parser(input, string_pool);
  parser.set_feed_open_token_callback([&result](HtmlToken&&, const char*) {
    ++result.tokens;
    return true;
  });
  parser.set_feed_text_token_callback([&result](HtmlTextToken&&) {
    ++result.tokens;
    return true;
  });
  parser.set_feed_close_token_callback([&result](HtmlCloseToken&&, const char*) {
    ++result.tokens;
    return true;
  });
  result.ok = parser.Parse();
  return result;
}

}  // namespace arboris::bench
