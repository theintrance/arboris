/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_TOKENS_HPP_
#define SRC_UTILS_TOKENS_HPP_

#include <cstdint>

namespace arboris {

// Base token structure for all token types
struct BaseToken {
  std::uint32_t begin_pos;
  std::uint32_t end_pos;
};

}  // namespace arboris

#endif  // SRC_UTILS_TOKENS_HPP_
