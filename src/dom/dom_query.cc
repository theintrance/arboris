/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_query.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <utility>
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
  std::optional<DOMQuery> found;
  forEachMatch(options, [&](const NodeRef& node) {
    found.emplace(node, subtree_);
    return false;
  });
  return found;
}

std::optional<DOMQuery> DOMQuery::Find(const std::string& id) const {
  auto node_key = subtree_.GetNodeById(id);
  if (!node_key) {
    return std::nullopt;
  }
  return DOMQuery(subtree_.GetNodeByKey(*node_key), subtree_);
}

std::vector<DOMQuery> DOMQuery::FindAll(const QueryOptions& options) const {
  std::vector<DOMQuery> found;
  forEachMatch(options, [&](const NodeRef& node) {
    found.emplace_back(node, subtree_);
    return true;
  });
  return found;
}

template <typename Visit>
void DOMQuery::forEachMatch(const QueryOptions& options, Visit&& visit) const {
  const auto check = [&](NodeKey key) {
    const auto node = subtree_.GetNodeByKey(key);
    return !matchAllConditions(node, options) || visit(node);
  };

  if (const auto candidates = indexedCandidates(options)) {
    std::ranges::all_of(*candidates, check);
  } else {
    std::ranges::all_of(subtree_.Descendants(), check);
  }
}

// The shortest index list among the conditions that have one, or nullopt when no condition
// has an index and every descendant is a candidate. An empty list settles the query.
std::optional<NodeKeySpan> DOMQuery::indexedCandidates(const QueryOptions& options) const {
  std::optional<NodeKeySpan> shortest;
  const auto consider = [&shortest](NodeKeySpan keys) {
    if (!shortest || keys.size() < shortest->size()) {
      shortest = keys;
    }
  };

  if (options.tag) {
    consider(subtree_.GetNodesByTag(*options.tag));
  }
  if (options.classes) {
    for (const auto& class_name : *options.classes) {
      consider(subtree_.GetNodesByClass(class_name));
    }
  }
  if (options.attributes) {
    for (const auto& [attribute_name, _] : *options.attributes) {
      consider(subtree_.GetNodesByAttribute(attribute_name));
    }
  }
  return shortest;
}

bool DOMQuery::matchAllConditions(const NodeRef& node, const QueryOptions& options) const {
  if (options.tag && node.tag() != *options.tag) {
    return false;
  }
  if (options.classes && !IsSubset(*options.classes, node.classes())) {
    return false;
  }
  if (options.attributes && !MatchesAttributes(*options.attributes, node.attributes())) {
    return false;
  }
  if (options.text && !options.text->Matches(node.text_content())) {
    return false;
  }
  return true;
}

}  // namespace arboris
