/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_query.hpp"

#include <optional>
#include <string>
#include <vector>

namespace arboris {

std::optional<DOMQuery> DOMQuery::Find(const QueryOptions& options) const {
  // TODO(team): Implement this
  return DOMQuery(root_, dom_indexer_);
}

std::optional<DOMQuery> DOMQuery::Find(const std::string& id) const {
  // TODO(team): Implement this
  NodePtr node = dom_indexer_.get().GetNodeById(id);
  if (node) {
    return DOMQuery(*node, dom_indexer_);
  }
  return std::nullopt;
}


std::vector<DOMQuery> DOMQuery::FindAll(const QueryOptions& options) const {
  std::vector<DOMQuery> ret;

  const auto& tag_filtered_list = dom_indexer_.get().GetNodesByTag(options.tag.value());

  return ret;
}

}  // namespace arboris
