/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_HTML_TOKENS_HPP_
#define SRC_UTILS_HTML_TOKENS_HPP_

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "utils/tag.hpp"
#include "utils/tokens.hpp"

namespace arboris {

struct BaseHtmlToken : public BaseToken {};

struct HtmlToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;
  bool is_void_tag = false;
  
  // TODO(team): Consider using string_views with an external string pool
  std::unordered_map<std::string, std::string> attributes;
  std::vector<std::string> classes;
  std::string id;
};

struct HtmlTextToken : public BaseHtmlToken {
  std::string_view text_content;
};

struct HtmlCloseToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;
};

}  // namespace arboris

#endif  // SRC_UTILS_HTML_TOKENS_HPP_
