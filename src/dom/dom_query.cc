/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_query.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "utils/html_tokens.hpp"
#include "utils/set_utils.hpp"

namespace arboris {

namespace {

// An attribute asked for with no value matches on its name alone, which is how a
// `[disabled]` style condition reads. A node carries few attributes, so its run is scanned
// rather than hashed.
bool MatchesAttributes(const AttributeMap& wanted, std::span<const Attribute> attributes) {
  for (const auto& [name, values] : wanted) {
    const auto found = std::ranges::find(attributes, name, &Attribute::name);
    if (found == attributes.end()) {
      return false;
    }
    if (!values.empty() && !values.contains(std::string(found->value))) {
      return false;
    }
  }
  return true;
}

}  // anonymous namespace

std::optional<DOMQuery> DOMQuery::Find(const QueryOptions& options) const {
  for (const auto& candidate_key : searchCandidatesFromSubtree(options)) {
    const auto candidate = subtree_.GetNodeByKey(candidate_key);
    if (matchAllConditions(candidate, options)) {
      return DOMQuery(candidate, subtree_);
    }
  }
  return std::nullopt;
}

std::optional<DOMQuery> DOMQuery::Find(const std::string& id) const {
  auto node_key = subtree_.GetNodeById(id);
  if (!node_key) {
    return std::nullopt;
  }
  return DOMQuery(subtree_.GetNodeByKey(*node_key), subtree_);
}

std::vector<DOMQuery> DOMQuery::FindAll(const QueryOptions& options) const {
  std::vector<DOMQuery> ret;

  for (const auto& candidate_key : searchCandidatesFromSubtree(options)) {
    const auto candidate = subtree_.GetNodeByKey(candidate_key);
    if (matchAllConditions(candidate, options)) {
      ret.push_back(DOMQuery(candidate, subtree_));
    }
  }
  return ret;
}

// The cheapest index wins: every candidate is checked against all conditions anyway, so
// the shortest list is the one worth walking.
NodeKeySpan DOMQuery::searchCandidatesFromSubtree(const QueryOptions& options) const {
  std::size_t min_size = std::numeric_limits<std::size_t>::max();
  NodeKeySpan min_candidates;

  const auto consider = [&min_size, &min_candidates](NodeKeySpan keys) {
    if (!keys.empty() && keys.size() < min_size) {
      min_size = keys.size();
      min_candidates = keys;
    }
  };

  if (options.tag.has_value()) {
    consider(subtree_.GetNodesByTag(options.tag.value()));
  }

  if (options.classes.has_value()) {
    for (const auto& class_name : *options.classes) {
      consider(subtree_.GetNodesByClass(class_name));
    }
  }

  if (options.attributes.has_value()) {
    for (const auto& [attribute_name, _] : options.attributes.value()) {
      consider(subtree_.GetNodesByAttribute(attribute_name));
    }
  }

  return min_candidates;
}

bool DOMQuery::matchAllConditions(const NodeRef& node, const QueryOptions& options) const {
  if (options.tag && node.tag() != options.tag.value()) {
    return false;
  }

  if (options.classes && !IsSubset(options.classes.value(), node.classes())) {
    return false;
  }

  if (options.attributes && !MatchesAttributes(options.attributes.value(), node.attributes())) {
    return false;
  }
  // TODO(team): Implement text condition matching
  return true;
}

}  // namespace arboris
