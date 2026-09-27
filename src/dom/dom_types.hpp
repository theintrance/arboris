/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_TYPES_HPP_
#define SRC_DOM_DOM_TYPES_HPP_

#include <cstdint>
#include <span>
#include <vector>

namespace arboris {


using NodeKey = std::uint32_t;
using NodeKeyList = std::vector<NodeKey>;
using NodeKeySpan = std::span<const NodeKey>;

}  // namespace arboris

#endif  // SRC_DOM_DOM_TYPES_HPP_

