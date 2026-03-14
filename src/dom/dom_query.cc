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
  auto candidate_keys = searchCandidatesFromSubtree(options);
  if (candidate_keys.empty()) {
    return std::nullopt;
  }

  for (const auto& candidate_key : candidate_keys) {
    const auto& candidate = subtree_.GetNodeByKey(candidate_key);
    if (matchAllConditions(candidate, options)) {
      return buildSubQuery(candidate_key);
    }
  }

  return std::nullopt;
}

std::optional<DOMQuery> DOMQuery::Find(const std::string& id) const {
  auto node_key = subtree_.GetNodeById(id);
  if (!node_key) {
    return std::nullopt;
  }
  return buildSubQuery(node_key.value());
}


std::vector<DOMQuery> DOMQuery::FindAll(const QueryOptions& options) const {
  std::vector<DOMQuery> ret;

  auto candidate_keys = searchCandidatesFromSubtree(options);
  for (auto candidate_key : candidate_keys) {
    const auto& candidate = subtree_.GetNodeByKey(candidate_key);
    if (matchAllConditions(candidate, options)) {
      ret.push_back(buildSubQuery(candidate_key));
    }
  }
  return ret;
}

NodeKeySpan DOMQuery::searchCandidatesFromSubtree(const QueryOptions& options) const {
  std::size_t min_size = std::numeric_limits<std::size_t>::max();
  NodeKeySpan min_candidates;

  if (options.tag.has_value()) {
    auto tag_index_keys = subtree_.GetNodesByTag(options.tag.value());
    if (tag_index_keys.has_value()) {
      if (tag_index_keys->size() < min_size) {
        min_size = tag_index_keys->size();
        min_candidates = *tag_index_keys;
      }
    }
  }

  if (options.classes.has_value()) {
    for (const auto& class_name : *options.classes) {
      auto class_index_keys = subtree_.GetNodesByClass(class_name);
      if (class_index_keys.has_value()) {
        if (class_index_keys->size() < min_size) {
          min_size = class_index_keys->size();
          min_candidates = *class_index_keys;
        }
      }
    }
  }

  if (options.attributes.has_value()) {
    for (const auto& [attribute_name, _] : options.attributes.value()) {
      auto attribute_index_keys = subtree_.GetNodesByAttribute(attribute_name);
      if (attribute_index_keys.has_value()) {
        if (attribute_index_keys->size() < min_size) {
          min_size = attribute_index_keys->size();
          min_candidates = *attribute_index_keys;
        }
      }
    }
  }

  return min_candidates;
}

bool DOMQuery::matchAllConditions(const TagNode& node, const QueryOptions& options) const {
  if (options.tag && node.tag() != options.tag.value()) {
    return false;
  }

  if (options.classes && !IsSubset(options.classes.value(), node.classes())) {
    return false;
  }

  if (options.attributes && !IsSubset(options.attributes.value(), node.attributes())) {
    return false;
  }
  // TODO(team): Implement text condition matching
  return true;
}

DOMQuery DOMQuery::buildSubQuery(NodeKey subtree_root_key) const {
  const auto& subtree_root = subtree_.GetNodeByKey(subtree_root_key);
  DOMSubtree subtree(subtree_, subtree_root);
  return DOMQuery(subtree);
}

}  // namespace arboris
