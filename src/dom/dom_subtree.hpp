/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_SUBTREE_HPP_
#define SRC_DOM_DOM_SUBTREE_HPP_

#include <cstdint>
#include <span>
#include <vector>

#include "dom/dom_types.hpp"
#include "dom/dom_indexer.hpp"
#include "utils/query_options.hpp"

namespace arboris {

class DOMSubtree {
 public:
  // Constructor for subtree of a parent subtree
  DOMSubtree(const DOMSubtree& parent, const TagNode& subtree_root);

  // Constructor for subtree of the root node
  DOMSubtree(
    const NodeList& global_dfs_node_list,
    const DOMIndexer& global_dom_indexer,
    const TagNode& subtree_root) :
      in_(subtree_root.in()),
      out_(subtree_root.out()),
      global_dfs_node_list_(global_dfs_node_list),
      global_dom_indexer_(global_dom_indexer) {}

  [[nodiscard]] std::optional<NodeKey> GetNodeById(std::string_view id) const;
  [[nodiscard]] std::optional<NodeKeySpan> GetNodesByTag(Tag tag) const;
  [[nodiscard]] std::optional<NodeKeySpan> GetNodesByClass(std::string_view class_name) const;
  [[nodiscard]] std::optional<NodeKeySpan> GetNodesByAttribute(std::string_view attribute_name) const;
  [[nodiscard]] const TagNode& GetNodeByKey(NodeKey node_key) const;

 private:
  bool isInSubtree(NodeKey node_key) const noexcept {
    return node_key >= in_ && node_key <= out_;
  }

  std::span<const NodeKey> sliceSubtreeRange(const NodeKeyList& node_keys) const noexcept {
    auto it  = std::lower_bound(node_keys.begin(), node_keys.end(), in_);
    auto end = std::upper_bound(node_keys.begin(), node_keys.end(), out_);
    return std::span<const NodeKey>(it, static_cast<std::size_t>(std::distance(it, end)));
  }

 private:
  const std::uint32_t in_;
  const std::uint32_t out_;

  const NodeList& global_dfs_node_list_;
  const DOMIndexer& global_dom_indexer_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_SUBTREE_HPP_
