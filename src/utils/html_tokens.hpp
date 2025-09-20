/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_HTML_TOKENS_HPP_
#define SRC_UTILS_HTML_TOKENS_HPP_

#include <cstdint>
#include <string_view>

#include "utils/tag.hpp"
#include "utils/tokens.hpp"

namespace arboris {

struct BaseHtmlToken : public BaseToken {
  BaseHtmlToken(std::uint32_t begin, std::uint32_t end) : BaseToken(TokenType::kHtml, begin, end) {}
};

struct HtmlToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;
  bool is_void_tag = false;
  // TODO(team): how to store the attributes such as href, src, etc.?

  HtmlToken(std::uint32_t begin, std::uint32_t end) : BaseHtmlToken(begin, end) {}
};

struct HtmlTextToken : public BaseHtmlToken {
  std::string_view text_content;

  HtmlTextToken(std::uint32_t begin, std::uint32_t end, std::string_view content)
      : BaseHtmlToken(begin, end), text_content(content) {}
};

struct HtmlCloseToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;

  HtmlCloseToken(std::uint32_t begin, std::uint32_t end) : BaseHtmlToken(begin, end) {}
};

}  // namespace arboris

#endif  // SRC_UTILS_HTML_TOKENS_HPP_
