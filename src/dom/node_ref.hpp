/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_NODE_REF_HPP_
#define SRC_DOM_NODE_REF_HPP_

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "dom/dom_store.hpp"
#include "dom/dom_types.hpp"
#include "dom/tag_node.hpp"
#include "utils/assertion.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// A handle to one node: a key and the store to resolve it against, 16 bytes, no ownership.
//
// Queries hand these out instead of the node itself, so the way nodes are stored stays
// behind this class. A key rather than a pointer, because the node array and the arenas
// grow while a document is parsed and every pointer into them moves with them.
class NodeRef {
 public:
  NodeRef(NodeKey key, const DOMStore& store) noexcept : key_(key), store_(&store) {}

  [[nodiscard]] NodeKey key() const noexcept {
    return key_;
  }

  [[nodiscard]] Tag tag() const noexcept {
    return node().tag();
  }

  // The tag name in lower case, "my-card" for a custom element; empty for the root.
  [[nodiscard]] std::string_view name() const noexcept {
    return node().name();
  }

  [[nodiscard]] std::string_view id() const noexcept {
    return node().id();
  }

  [[nodiscard]] std::string_view text_content() const noexcept {
    return node().text_content();
  }

  [[nodiscard]] std::uint32_t sub_tree_size() const noexcept {
    return node().sub_tree_size();
  }

  [[nodiscard]] std::span<const std::string_view> classes() const noexcept {
    const auto& tag_node = node();
    return {store_->class_arena().data() + tag_node.class_begin(), tag_node.class_count()};
  }

  // The text written directly inside this tag, in document order. Text further down sits
  // on the node that holds it.
  [[nodiscard]] std::span<const TextRun> text_runs() const noexcept {
    const auto& tag_node = node();
    return {store_->text_arena().data() + tag_node.text_begin(), tag_node.text_count()};
  }

  [[nodiscard]] std::span<const Attribute> attributes() const noexcept {
    const auto& tag_node = node();
    return {store_->attr_arena().data() + tag_node.attr_begin(), tag_node.attr_count()};
  }

  [[nodiscard]] NodeRef parent() const noexcept {
    return NodeRef(node().parent_key(), *store_);
  }

  // The first element under this one, if any: the next key, since keys run in DFS order.
  [[nodiscard]] std::optional<NodeRef> first_child() const noexcept {
    if (node().sub_tree_size() <= 1) {
      return std::nullopt;
    }
    return NodeRef(key_ + 1, *store_);
  }

  [[nodiscard]] bool is_first_child() const noexcept {
    const auto& tag_node = node();
    return tag_node.parent_key() != key_ && tag_node.parent_key() + 1 == key_;
  }

  // The element after this one under the same parent, if any. The root has no siblings.
  [[nodiscard]] std::optional<NodeRef> next_sibling() const noexcept {
    const auto& tag_node = node();
    if (tag_node.parent_key() == key_) {
      return std::nullopt;
    }
    const auto& parent_node = store_->nodes()[tag_node.parent_key()];
    const NodeKey next = key_ + std::max(1U, tag_node.sub_tree_size());
    if (next >= parent_node.key() + parent_node.sub_tree_size()) {
      return std::nullopt;
    }
    return NodeRef(next, *store_);
  }

  // The element before this one under the same parent, if any. In DFS order the key just
  // before this one is that sibling or its last descendant, so it is reached by climbing
  // from there until the parent is shared: O(depth), not O(siblings).
  [[nodiscard]] std::optional<NodeRef> previous_sibling() const noexcept {
    const NodeKey parent_key = node().parent_key();
    if (parent_key == key_ || parent_key + 1 == key_) {
      return std::nullopt;
    }
    NodeKey candidate = key_ - 1;
    while (store_->nodes()[candidate].parent_key() != parent_key) {
      candidate = store_->nodes()[candidate].parent_key();
    }
    return NodeRef(candidate, *store_);
  }

  // Children are found by walking keys, not by keeping a list: the first child is the next
  // key, and each sibling sits one whole subtree further on.
  [[nodiscard]] std::vector<NodeRef> children() const {
    std::vector<NodeRef> out;
    const auto& tag_node = node();
    const NodeKey end = key_ + tag_node.sub_tree_size();
    for (NodeKey child = key_ + 1; child < end;) {
      out.emplace_back(child, *store_);
      // Never step by nothing: a size of 0 would loop here forever.
      child += std::max(1U, store_->nodes()[child].sub_tree_size());
    }
    return out;
  }

  [[nodiscard]] const TagNode& node() const noexcept {
    ARBORIS_ASSERT(key_ < store_->nodes().size(), "Node key is out of range.");
    return store_->nodes()[key_];
  }

 private:
  NodeKey key_;
  const DOMStore* store_;
};

}  // namespace arboris

#endif  // SRC_DOM_NODE_REF_HPP_
