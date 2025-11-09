/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>

#include "dom/dom_indexer.hpp"

namespace arboris {

void DOMIndexer::AddNode(const std::shared_ptr<TagNode>& node) {
  tag_index_[node->tag()].emplace_back(node);
  // TODO(team): add class index
  // TODO(team): add id index
}

}  // namespace arboris
