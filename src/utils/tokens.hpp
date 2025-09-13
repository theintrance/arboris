/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_TOKENS_HPP_
#define SRC_UTILS_TOKENS_HPP_

#include <string_view>
#include "utils/tag.h"

namespace arboris {

struct BaseToken {
};

struct OpenToken : public BaseToken {
    Tag tag = Tag::kUnknown;
    // TODO(team): how to store the attributes such as href, src, etc.?
};

struct TextToken : public BaseToken {
    std::string_view text;
};

struct CloseToken : public BaseToken {
    Tag tag = Tag::kUnknown;
};

}  // namespace arboris

#endif  // SRC_UTILS_TOKENS_HPP_
