/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_subtree.hpp"

#include <optional>
#include <string_view>

#include "utils/assertion.hpp"

namespace arboris {

DOMSubtree::DOMSubtree(const DOMSubtree& parent, const TagNode& subtree_root)
    : root_key_(subtree_root.key()),
      sub_tree_size_(subtree_root.sub_tree_size()),
      store_(parent.store_) {
  ARBORIS_ASSERT(subtree_root.key() >= parent.root_key_ &&
                 subtree_root.key() < parent.root_key_ + parent.sub_tree_size_,
                 "Subtree root must be a child of the parent.");
}

std::optional<NodeKey> DOMSubtree::GetNodeById(std::string_view id) const {
  auto node_key = store_->indexer().GetNodeKeyById(id);
  if (!node_key || !isInSubtree(*node_key)) {
    return std::nullopt;
  }
  return node_key.value();
}

NodeKeySpan DOMSubtree::GetNodesByTag(Tag tag) const {
  return sliceSubtreeRange(store_->indexer().GetNodeKeyListByTag(tag));
}

NodeKeySpan DOMSubtree::GetNodesByClass(std::string_view class_name) const {
  return sliceSubtreeRange(store_->indexer().GetNodeKeyListByClass(class_name));
}

NodeKeySpan DOMSubtree::GetNodesByAttribute(std::string_view attribute_name) const {
  return sliceSubtreeRange(store_->indexer().GetNodeKeyListByAttribute(attribute_name));
}

}  // namespace arboris
