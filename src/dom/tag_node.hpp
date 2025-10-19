/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TAG_NODE_HPP_
#define SRC_DOM_TAG_NODE_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dom/base_node.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

class TagNode final : public BaseNode {
 public:
  explicit TagNode(std::uint32_t id, HtmlToken&& token, std::shared_ptr<BaseNode> parent = nullptr)
      : BaseNode(NodeType::kTag, id, std::move(parent)), html_token_(std::move(token)) {}

  const std::vector<std::shared_ptr<BaseNode>>& Children() const noexcept override {
    return children_;
  }

  const std::unordered_map<std::string, std::string>& Attributes() const noexcept override {
    return html_token_.attributes;
  }

  const std::vector<std::string>& Classes() const noexcept override {
    return html_token_.classes;
  }

  std::string_view Id() const noexcept override {
    return html_token_.id;
  }

  void AddChild(std::shared_ptr<BaseNode> child) {
    ARBORIS_ASSERT(child != nullptr, "child must not be nullptr.");
    children_.emplace_back(std::move(child));
  }

 private:
  const HtmlToken html_token_;
  std::vector<std::shared_ptr<BaseNode>> children_;
};

}  // namespace arboris

#endif  // SRC_DOM_TAG_NODE_HPP_
