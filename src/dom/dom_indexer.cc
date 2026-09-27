/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_indexer.hpp"

#include <optional>
#include <string_view>
#include <unordered_map>

#include "dom/dom_types.hpp"
#include "dom/node_ref.hpp"

namespace arboris {

namespace {

NodeKeySpan Lookup(const std::unordered_map<std::string_view, NodeKeyList>& index,
                   std::string_view key) {
  auto it = index.find(key);
  return it != index.end() ? NodeKeySpan{it->second} : NodeKeySpan{};
}

}  // anonymous namespace

void DOMIndexer::AddNode(const NodeRef& node) {
  const NodeKey key = node.key();

  tag_index_[node.tag()].emplace_back(key);

  for (const auto& class_name : node.classes()) {
    class_index_[class_name].emplace_back(key);
  }

  for (const auto& attribute : node.attributes()) {
    attr_index_[attribute.name].emplace_back(key);
  }

  // The first id wins, as querySelector does with a repeated id.
  if (!node.id().empty()) {
    id_index_.try_emplace(node.id(), key);
  }
}

std::optional<NodeKey> DOMIndexer::GetNodeKeyById(std::string_view id) const {
  auto it = id_index_.find(id);
  return it != id_index_.end() ? std::make_optional(it->second) : std::nullopt;
}

NodeKeySpan DOMIndexer::GetNodeKeyListByTag(Tag tag) const {
  auto it = tag_index_.find(tag);
  return it != tag_index_.end() ? NodeKeySpan{it->second} : NodeKeySpan{};
}

NodeKeySpan DOMIndexer::GetNodeKeyListByClass(std::string_view class_name) const {
  return Lookup(class_index_, class_name);
}

NodeKeySpan DOMIndexer::GetNodeKeyListByAttribute(std::string_view attribute_name) const {
  return Lookup(attr_index_, attribute_name);
}

}  // namespace arboris
