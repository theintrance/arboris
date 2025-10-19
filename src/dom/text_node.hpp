/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TEXT_NODE_HPP_
#define SRC_DOM_TEXT_NODE_HPP_

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

#include "dom/base_node.hpp"

namespace arboris {

class TextNode final : public BaseNode {
 public:
  explicit TextNode(std::uint32_t id, std::shared_ptr<BaseNode> parent = nullptr)
      : BaseNode(NodeType::kText, id, std::move(parent)) {}

  const std::vector<std::shared_ptr<BaseNode>>& Children() const noexcept override {
    static const std::vector<std::shared_ptr<BaseNode>> kEmptyChildren;
    return kEmptyChildren;
  };

  const std::unordered_map<std::string, std::string>& Attributes() const noexcept override {
    static const std::unordered_map<std::string, std::string> kEmptyAttributes;
    return kEmptyAttributes;
  };

  const std::vector<std::string>& Classes() const noexcept override {
    static const std::vector<std::string> kEmptyClasses;
    return kEmptyClasses;
  };

  std::string_view Id() const noexcept override {
    return {};
  };
};

}  // namespace arboris

#endif  // SRC_DOM_TEXT_NODE_HPP_
