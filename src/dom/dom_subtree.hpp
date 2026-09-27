/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_SUBTREE_HPP_
#define SRC_DOM_DOM_SUBTREE_HPP_

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string_view>

#include "dom/dom_store.hpp"
#include "dom/dom_types.hpp"
#include "dom/node_ref.hpp"
#include "dom/tag_node.hpp"

namespace arboris {

// A key range of the document: nodes are numbered in DFS order, so one subtree is
// [root_key_, root_key_ + sub_tree_size_). Index hits are sliced down to that range.
class DOMSubtree {
 public:
  // Constructor for subtree of a parent subtree
  DOMSubtree(const DOMSubtree& parent, const TagNode& subtree_root);

  // Constructor for subtree of the root node
  DOMSubtree(const DOMStore& store, const TagNode& subtree_root) :
      root_key_(subtree_root.key()),
      sub_tree_size_(subtree_root.sub_tree_size()),
      store_(&store) {}

  [[nodiscard]] std::optional<NodeKey> GetNodeById(std::string_view id) const;
  [[nodiscard]] NodeKeySpan GetNodesByTag(Tag tag) const;
  [[nodiscard]] NodeKeySpan GetNodesByClass(std::string_view class_name) const;
  [[nodiscard]] NodeKeySpan GetNodesByAttribute(std::string_view attribute_name) const;

  [[nodiscard]] NodeRef GetNodeByKey(NodeKey node_key) const {
    ARBORIS_ASSERT(isInSubtree(node_key), "Node key must be in subtree.");
    return NodeRef(node_key, *store_);
  }

  [[nodiscard]] const DOMStore& store() const noexcept {
    return *store_;
  }

 private:
  [[nodiscard]] bool isInSubtree(NodeKey key) const noexcept {
    return key >= root_key_ && key < root_key_ + sub_tree_size_;
  }

  [[nodiscard]] NodeKeySpan sliceSubtreeRange(NodeKeySpan keys) const noexcept {
    const auto subtree_begin = root_key_;
    const auto subtree_end = root_key_ + sub_tree_size_;
    const auto first = std::lower_bound(keys.begin(), keys.end(), subtree_begin);
    const auto last = std::lower_bound(first, keys.end(), subtree_end);
    return {first, static_cast<std::size_t>(last - first)};
  }

 private:
  NodeKey root_key_;
  std::uint32_t sub_tree_size_;

  const DOMStore* store_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_SUBTREE_HPP_
