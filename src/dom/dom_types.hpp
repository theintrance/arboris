/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_TYPES_HPP_
#define SRC_DOM_DOM_TYPES_HPP_

#include <memory>
#include <vector>

namespace arboris {

class TagNode;

using NodePtr = std::shared_ptr<TagNode>;
using NodeList = std::vector<NodePtr>;

}  // namespace arboris

#endif  // SRC_DOM_DOM_TYPES_HPP_

