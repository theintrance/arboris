/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_indexer.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "dom/dom_types.hpp"
#include "dom/node_ref.hpp"

namespace arboris {

void DOMIndexer::AddNode(const NodeRef& node) {
  const NodeKey node_key = node.key();

  tag_index_[node.tag()].emplace_back(node_key);

  for (const auto& class_name : node.classes()) {
    class_index_[class_name].emplace_back(node_key);
  }
  // TODO(team): add id index
}

std::optional<NodeKey> DOMIndexer::GetNodeKeyById(std::string_view id) const {
  // TODO(team): consider heterogeneous lookup to avoid std::string allocation
  auto it = id_index_.find(std::string(id));
  return it != id_index_.end() ? std::make_optional(it->second) : std::nullopt;
}

NodeKeySpan DOMIndexer::GetNodeKeyListByTag(Tag tag) const {
  auto it = tag_index_.find(tag);
  return it != tag_index_.end() ? NodeKeySpan{it->second} : NodeKeySpan{};
}

NodeKeySpan DOMIndexer::GetNodeKeyListByClass(std::string_view class_name) const {
  auto it = class_index_.find(std::string(class_name));
  return it != class_index_.end() ? NodeKeySpan{it->second} : NodeKeySpan{};
}

NodeKeySpan DOMIndexer::GetNodeKeyListByAttribute(std::string_view attribute_name) const {
  auto it = attr_index_.find(std::string(attribute_name));
  return it != attr_index_.end() ? NodeKeySpan{it->second} : NodeKeySpan{};
}

}  // namespace arboris
