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

namespace arboris {

void DOMIndexer::AddNode(const TagNode& node) {
  auto locator = NodeLocator{node.key(), node.in()};
  tag_index_[node.tag()].emplace_back(locator);
  for (const auto& class_name : node.classes()) {
    class_index_[class_name].emplace_back(locator);
  }
  // id_index_[std::string(node.id())] = node.key();
  for (const auto& [attribute_name, attribute_value] : node.attributes()) {
    attr_index_.emplace(attribute_name, locator);
  }
}

std::optional<NodeLocator> DOMIndexer::GetNodeKeyById(std::string_view id) const {
  // TODO(team): consider heterogeneous lookup to avoid std::string allocation
  auto it = id_index_.find(std::string(id));
  return it != id_index_.end() ? std::make_optional<NodeLocator>(it->second) : std::nullopt;
}

const NodeLocatorList* DOMIndexer::GetNodeKeyListByTag(Tag tag) const {
  auto it = tag_index_.find(tag);
  return it != tag_index_.end() ? &it->second : nullptr;
}

const NodeLocatorList* DOMIndexer::GetNodeKeyListByClass(std::string_view class_name) const {
  auto it = class_index_.find(std::string(class_name));
  return it != class_index_.end() ? &it->second : nullptr;
}

const NodeLocatorList* DOMIndexer::GetNodeKeyListByAttribute(std::string_view attribute_name) const {
  auto it = attr_index_.find(std::string(attribute_name));
  return it != attr_index_.end() ? &it->second : nullptr;
}

}  // namespace arboris
