/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_HTML_TOKENS_HPP_
#define SRC_UTILS_HTML_TOKENS_HPP_

#include <string_view>
#include "utils/tag.h"

namespace arboris {

struct BaseHtmlToken {
    std::uint32_t begin_pos;
    std::uint32_t end_pos;
};

struct HtmlToken : public BaseHtmlToken {
    Tag tag = Tag::kUnknown;
    bool is_void_tag = false;
    // TODO(team): how to store the attributes such as href, src, etc.?
};

struct HtmlTextToken : public BaseHtmlToken {
    std::string_view text_content;
};

struct HtmlCloseToken : public BaseHtmlToken {
    Tag tag = Tag::kUnknown;
};

}  // namespace arboris

#endif  // SRC_UTILS_HTML_TOKENS_HPP_
