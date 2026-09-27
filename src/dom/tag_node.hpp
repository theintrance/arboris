/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TAG_NODE_HPP_
#define SRC_DOM_TAG_NODE_HPP_

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/dom_types.hpp"
#include "utils/assertion.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// A node knows where its classes, attributes and text sit, not what they say: those live
// in DOMStore's arenas, and NodeRef reads them back.
//
// Nodes are values in one array, so this has to move; nothing here owns anything, which is
// what makes the move trivial.
class TagNode {
 public:
  TagNode(NodeKey key, NodeKey parent_key, HtmlToken&& token)
      : key_(key), parent_key_(parent_key), html_token_(std::move(token)) {}

  TagNode(const TagNode&) = delete;
  TagNode& operator=(const TagNode&) = delete;
  TagNode(TagNode&&) = default;
  TagNode& operator=(TagNode&&) = delete;
  ~TagNode() = default;

  [[nodiscard]] std::string_view text_content() const noexcept {
    return text_content_;
  }

  void set_text_content(std::string_view text_content) noexcept {
    text_content_ = text_content;
  }

  [[nodiscard]] NodeKey key() const noexcept {
    return key_;
  }

  // The root is its own parent, so walking up always terminates.
  [[nodiscard]] NodeKey parent_key() const noexcept {
    return parent_key_;
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

 private:
  NodeKey key_;
  NodeKey parent_key_;
  std::string_view text_content_;
  std::uint32_t sub_tree_size_{0};
  std::uint32_t text_begin_{0};
  std::uint32_t text_count_{0};
  HtmlToken html_token_;
};

// Nodes live in one array, in DFS order, indexed by key.
using TagNodeList = std::vector<TagNode>;

}  // namespace arboris

#endif  // SRC_DOM_TAG_NODE_HPP_
