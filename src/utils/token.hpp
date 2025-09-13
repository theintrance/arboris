/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_TOKEN_HPP_
#define SRC_UTILS_TOKEN_HPP_

#include <string_view>
#include "utils/tag.h"

namespace arboris {

struct Token {
    Tags tag = Tags::kUnknown;
    std::string_view text_content;
    // TODO(team): how to store the attributes such as href, src, etc.?
};

}  // namespace arboris

#endif  // SRC_UTILS_TOKEN_HPP_
