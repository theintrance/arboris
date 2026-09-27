/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TAG_NODE_HPP_
#define SRC_DOM_TAG_NODE_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/dom_types.hpp"
#include "dom/base_node.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// A node knows where its classes and attributes sit, not what they say: the names and
// values live in DOMStore's arenas. NodeRef is what reads them back.
class TagNode final : public BaseNode {
 public:
  static constexpr NodeType kNodeType = NodeType::kTag;

  explicit TagNode(NodeKey key, HtmlToken&& token, const std::shared_ptr<TagNode> parent)
      : BaseNode(kNodeType, parent), key_(key), html_token_(std::move(token)) {}

  [[nodiscard]] NodeKey key() const noexcept {
    return key_;
  }

  [[nodiscard]] const std::vector<std::shared_ptr<TagNode>>& children() const noexcept {
    return children_;
  }

  [[nodiscard]] std::string_view id() const noexcept {
    return html_token_.id;
  }

  [[nodiscard]] Tag tag() const noexcept {
    return html_token_.tag;
  }

  [[nodiscard]] std::uint32_t class_begin() const noexcept {
    return html_token_.class_begin;
  }

  [[nodiscard]] std::uint32_t class_count() const noexcept {
    return html_token_.class_count;
  }

  [[nodiscard]] std::uint32_t attr_begin() const noexcept {
    return html_token_.attr_begin;
  }

  [[nodiscard]] std::uint32_t attr_count() const noexcept {
    return html_token_.attr_count;
  }

  [[nodiscard]] std::uint32_t text_begin() const noexcept {
    return text_begin_;
  }

  [[nodiscard]] std::uint32_t text_count() const noexcept {
    return text_count_;
  }

  void set_text_runs(std::uint32_t begin, std::uint32_t count) noexcept {
    text_begin_ = begin;
    text_count_ = count;
  }

  void set_sub_tree_size(std::uint32_t size) noexcept {
    sub_tree_size_ = size;
  }

  [[nodiscard]] std::uint32_t sub_tree_size() const noexcept {
    return sub_tree_size_;
  }

  void AddChild(std::shared_ptr<TagNode> child) {
    ARBORIS_ASSERT(child != nullptr, "child must not be nullptr.");
    children_.emplace_back(std::move(child));
  }

 private:
  const NodeKey key_;
  std::uint32_t sub_tree_size_{0};
  std::uint32_t text_begin_{0};
  std::uint32_t text_count_{0};
  const HtmlToken html_token_;
  std::vector<std::shared_ptr<TagNode>> children_;
};

}  // namespace arboris

#endif  // SRC_DOM_TAG_NODE_HPP_
