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
#include <string_view>
#include <utility>
#include <vector>

#include "string/string.hpp"
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

bool HasAsciiUpper(std::string_view text) {
  return std::ranges::any_of(text, IsAsciiUpper);
}

// Names are stored lower-case, so a condition written with capitals is lowered once here
// rather than compared ignoring case at every node. Nullopt when nothing needs lowering.
std::optional<QueryOptions> LowerCasedNames(const QueryOptions& options) {
  const bool name_needs_it = options.name && HasAsciiUpper(*options.name);
  const bool attributes_need_it = options.attributes && std::ranges::any_of(*options.attributes, [](const auto& entry) {
    return HasAsciiUpper(entry.first);
  });
  if (!name_needs_it && !attributes_need_it) {
    return std::nullopt;
  }

  QueryOptions lowered = options;
  if (name_needs_it) {
    lowered.name = AsciiLowerCase(*options.name);
  }
  if (attributes_need_it) {
    AttributeMap attributes;
    for (const auto& [name, values] : *options.attributes) {
      attributes[AsciiLowerCase(name)].insert(values.begin(), values.end());
    }
    lowered.attributes = std::move(attributes);
  }
  return lowered;
}

// Keeps the shortest of the index lists offered to it.
class ShortestSpan {
 public:
  void Consider(NodeKeySpan keys) {
    if (!shortest_ || keys.size() < shortest_->size()) {
      shortest_ = keys;
    }
  }

  [[nodiscard]] std::optional<NodeKeySpan> get() const noexcept {
    return shortest_;
  }

 private:
  std::optional<NodeKeySpan> shortest_;
};

}  // anonymous namespace

std::optional<DOMQuery> DOMQuery::Find(const QueryOptions& options) const {
  std::optional<DOMQuery> found;
  forEachQueryMatch(options, [&](const NodeRef& node) {
    found.emplace(node, subtree_);
    return false;
  });
  return found;
}

std::optional<DOMQuery> DOMQuery::Find(const std::string& id) const {
  const auto keys = subtree_.GetNodesById(id);
  if (keys.empty()) {
    return std::nullopt;
  }
  return DOMQuery(subtree_.GetNodeByKey(keys.front()), subtree_);
}

std::vector<DOMQuery> DOMQuery::FindAll(const QueryOptions& options) const {
  std::vector<DOMQuery> found;
  forEachQueryMatch(options, [&](const NodeRef& node) {
    found.emplace_back(node, subtree_);
    return true;
  });
  return found;
}

template <typename Matches, typename Visit>
void DOMQuery::forEachMatch(std::optional<NodeKeySpan> candidates, Matches&& matches, Visit&& visit) const {
  const auto check = [&](NodeKey key) {
    const auto node = subtree_.GetNodeByKey(key);
    return !matches(node) || visit(node);
  };

  if (candidates) {
    for (const NodeKey key : *candidates) {
      if (!check(key)) {
        return;
      }
    }
  } else {
    for (const NodeKey key : subtree_.Descendants()) {
      if (!check(key)) {
        return;
      }
    }
  }
}

template <typename Visit>
void DOMQuery::forEachQueryMatch(const QueryOptions& given, Visit&& visit) const {
  const std::optional<QueryOptions> lowered = LowerCasedNames(given);
  const QueryOptions& options = lowered ? *lowered : given;
  forEachMatch(
      indexedCandidates(options), [&](const NodeRef& node) { return matchAllConditions(node, options); },
      std::forward<Visit>(visit));
}

std::optional<DOMQuery> DOMQuery::Select(const Selector& selector) const {
  const auto keys = selectKeys(selector, true);
  if (keys.empty()) {
    return std::nullopt;
  }
  return DOMQuery(subtree_.GetNodeByKey(keys.front()), subtree_);
}

std::optional<DOMQuery> DOMQuery::Select(std::string_view selector) const {
  const auto parsed = Selector::Parse(selector);
  return parsed ? Select(*parsed) : std::nullopt;
}

std::vector<DOMQuery> DOMQuery::SelectAll(const Selector& selector) const {
  std::vector<DOMQuery> found;
  for (const NodeKey key : selectKeys(selector, false)) {
    found.emplace_back(subtree_.GetNodeByKey(key), subtree_);
  }
  return found;
}

std::vector<DOMQuery> DOMQuery::SelectAll(std::string_view selector) const {
  const auto parsed = Selector::Parse(selector);
  return parsed ? SelectAll(*parsed) : std::vector<DOMQuery>{};
}

// Each alternative is tried from the index its subject compound allows. One alternative
// already yields document order; several are merged into it. Only the subject needs to be
// a descendant: the elements the combinators lead to may sit above the subtree, as they do
// for querySelectorAll.
std::vector<NodeKey> DOMQuery::selectKeys(const Selector& selector, bool first_only) const {
  std::vector<NodeKey> keys;
  for (const auto& complex : selector.alternatives()) {
    if (complex.compounds.empty()) {
      continue;
    }
    ComplexMatcher matcher(complex);
    forEachMatch(
        indexedCandidates(complex.compounds.front()),
        [&](const NodeRef& node) { return matcher.Matches(node); },
        [&](const NodeRef& node) {
          keys.push_back(node.key());
          return !first_only;
        });
  }

  if (selector.alternatives().size() > 1) {
    std::ranges::sort(keys);
    const auto [first, last] = std::ranges::unique(keys);
    keys.erase(first, last);
  }
  return keys;
}

std::optional<NodeKeySpan> DOMQuery::indexedCandidates(const QueryOptions& options) const {
  ShortestSpan shortest;
  if (options.tag) {
    shortest.Consider(subtree_.GetNodesByTag(*options.tag));
  }
  if (options.name) {
    shortest.Consider(subtree_.GetNodesByName(*options.name));
  }
  if (options.classes) {
    for (const auto& class_name : *options.classes) {
      shortest.Consider(subtree_.GetNodesByClass(class_name));
    }
  }
  if (options.attributes) {
    for (const auto& [attribute_name, _] : *options.attributes) {
      shortest.Consider(subtree_.GetNodesByAttribute(attribute_name));
    }
  }
  return shortest.get();
}

std::optional<NodeKeySpan> DOMQuery::indexedCandidates(const CompoundSelector& compound) const {
  ShortestSpan shortest;
  if (compound.id) {
    shortest.Consider(subtree_.GetNodesById(*compound.id));
  }
  if (compound.tag) {
    shortest.Consider(subtree_.GetNodesByName(*compound.tag));
  }
  for (const auto& class_name : compound.classes) {
    shortest.Consider(subtree_.GetNodesByClass(class_name));
  }
  for (const auto& attribute : compound.attributes) {
    shortest.Consider(subtree_.GetNodesByAttribute(attribute.name));
  }
  return shortest.get();
}

bool DOMQuery::matchAllConditions(const NodeRef& node, const QueryOptions& options) const {
  if (options.tag && node.tag() != *options.tag) {
    return false;
  }
  if (options.name && node.name() != *options.name) {
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
