/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_subtree.hpp"
#include "utils/assertion.hpp"

namespace arboris {

DOMSubtree::DOMSubtree(const DOMSubtree& parent, const TagNode& subtree_root)
    : in_(subtree_root.in()),
      out_(subtree_root.out()),
      global_dfs_node_list_(parent.global_dfs_node_list_),
      global_dom_indexer_(parent.global_dom_indexer_) {

  ARBORIS_ASSERT(subtree_root.in_ >= parent.in_ && subtree_root.out_ <= parent.out_,
                 "Subtree root must be a descendant of the parent.");
}

std::optional<NodeKey> DOMSubtree::GetNodeById(std::string_view id) const {
  auto node_key = global_dom_indexer_.GetNodeKeyById(id);
  if (!node_key || !isInSubtree(*node_key)) {
    return std::nullopt;
  }
  return node_key.value();
}

std::optional<NodeKeySpan> DOMSubtree::GetNodesByTag(Tag tag) const {
  const auto& node_keys = global_dom_indexer_.GetNodeKeyListByTag(tag);
  if (!node_keys || node_keys->empty()) {
    return std::nullopt;
  }
  return sliceSubtreeRange(*node_keys);
}

std::optional<NodeKeySpan> DOMSubtree::GetNodesByClass(std::string_view class_name) const {
  const auto& node_keys = global_dom_indexer_.GetNodeKeyListByClass(class_name);
  if (!node_keys || node_keys->empty()) {
    return std::nullopt;
  }
  return sliceSubtreeRange(*node_keys);
}

std::optional<NodeKeySpan> DOMSubtree::GetNodesByAttribute(std::string_view attribute_name) const {
  const auto& node_keys = global_dom_indexer_.GetNodeKeyListByAttribute(attribute_name);
  if (!node_keys || node_keys->empty()) {
    return std::nullopt;
  }
  return sliceSubtreeRange(*node_keys);
}

const TagNode& DOMSubtree::GetNodeByKey(NodeKey node_key) const {
  ARBORIS_ASSERT(isInSubtree(node_key), "Node key must be in subtree.");
  return *global_dfs_node_list_[node_key];
}

}  // namespace arboris
