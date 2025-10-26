/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TEXT_NODE_HPP_
#define SRC_DOM_TEXT_NODE_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dom/base_node.hpp"

namespace arboris {

class TextNode final : public BaseNode {
  static constexpr NodeType kNodeType = NodeType::kText;

 public:
  explicit TextNode(std::uint32_t node_id, std::shared_ptr<BaseNode> parent)
      : BaseNode(kNodeType, node_id, std::move(parent)) {}
};

}  // namespace arboris

#endif  // SRC_DOM_TEXT_NODE_HPP_
