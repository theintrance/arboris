/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_query.hpp"

#include <optional>
#include <string>
#include <limits>
#include <vector>
#include <utility>

#include "utils/set_utils.hpp"

namespace arboris {

std::optional<DOMQuery> DOMQuery::Find(const QueryOptions& options) const {
  // TODO(team): Implement this
  auto candidates = searchCandidatesFromIndexer(options);
  if (candidates.empty()) {
    return std::nullopt;
  }

  for (const auto& candidate : candidates) {
    if (matchAllConditions(candidate, options)) {
      return DOMQuery(*candidate, dom_indexer_);
    }
  }

  return std::nullopt;
}

std::optional<DOMQuery> DOMQuery::Find(const std::string& id) const {
  NodePtr node = dom_indexer_.get().GetNodeById(id);
  if (node) {
    return DOMQuery(*node, dom_indexer_);
  }
  return std::nullopt;
}


std::vector<DOMQuery> DOMQuery::FindAll(const QueryOptions& options) const {
  std::vector<DOMQuery> ret;

  auto candidates = searchCandidatesFromIndexer(options);
  for (const auto& candidate : candidates) {
    if (matchAllConditions(candidate, options)) {
      ret.push_back(DOMQuery(*candidate, dom_indexer_));
    }
  }
  return ret;
}

NodeList DOMQuery::searchCandidatesFromIndexer(const QueryOptions& options) const {
  std::size_t min_size = std::numeric_limits<std::size_t>::max();
  NodeList min_candidates;

  if (options.tag.has_value()) {
    auto nodes = dom_indexer_.get().GetNodesByTag(options.tag.value());
    if (nodes.has_value()) {
      if (nodes->size() < min_size) {
        min_size = nodes->size();
        min_candidates = std::move(*nodes);
      }
    }
  }

  if (options.classes.has_value()) {
    for (const auto& class_name : *options.classes) {
      auto nodes = dom_indexer_.get().GetNodesByClass(class_name);
      if (nodes.has_value()) {
        if (nodes->size() < min_size) {
          min_size = nodes->size();
          min_candidates = std::move(*nodes);
        }
      }
    }
  }

  if (options.attributes.has_value()) {
    for (const auto& [attribute_name, _] : options.attributes.value()) {
      auto nodes = dom_indexer_.get().GetNodesByAttribute(attribute_name);
      if (nodes.has_value()) {
        if (nodes->size() < min_size) {
          min_size = nodes->size();
          min_candidates = std::move(*nodes);
        }
      }
    }
  }

  if (min_candidates.empty()) {
    return {};
  }

  return min_candidates;
}

bool DOMQuery::matchAllConditions(const NodePtr& node, const QueryOptions& options) const {
  if (!isSubNode(node)) {
    return false;
  }

  if (options.tag && node->tag() != options.tag.value()) {
    return false;
  }

  if (options.classes && !IsSubset(options.classes.value(), node->classes())) {
    return false;
  }

  if (options.attributes && !IsSubset(options.attributes.value(), node->attributes())) {
    return false;
  }
  // TODO(team): Implement text condition matching
  return true;
}

bool DOMQuery::isSubNode(const NodePtr& node) const {
  const uint32_t node_in = node->in();
  const uint32_t node_out = node->out();
  const uint32_t root_in = root_.get().in();
  const uint32_t root_out = root_.get().out();
  return node_in >= root_in && node_out <= root_out;
}

}  // namespace arboris
