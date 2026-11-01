/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_QUERY_HPP_
#define SRC_DOM_DOM_QUERY_HPP_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dom/dom_subtree.hpp"
#include "dom/dom_types.hpp"
#include "dom/node_ref.hpp"
#include "selector/selector.hpp"
#include "utils/query_options.hpp"

namespace arboris {

// One result of a query, and the subtree the next query runs over.
//
// Get() hands back a NodeRef rather than the node itself, so how nodes are stored stays
// an implementation detail of DOMStore.
class DOMQuery {
 public:
  explicit DOMQuery(const NodeRef& subtree_root, const DOMSubtree& subtree) :
    subtree_root_(subtree_root), subtree_(subtree, subtree_root.node()) {}

  DOMQuery(const DOMQuery&) = default;
  DOMQuery& operator=(const DOMQuery&) = delete;
  DOMQuery(DOMQuery&&) = default;
  DOMQuery& operator=(DOMQuery&&) = delete;
  virtual ~DOMQuery() = default;

  [[nodiscard]] NodeRef Get() const noexcept {
    return subtree_root_;
  }

  [[nodiscard]] std::optional<DOMQuery> Find(const QueryOptions& options) const;
  [[nodiscard]] std::optional<DOMQuery> Find(const std::string& id) const;
  [[nodiscard]] std::vector<DOMQuery> FindAll(const QueryOptions& options) const;

  // The descendants a CSS selector matches, in document order. The text overloads parse
  // the selector first; text that is not a selector matches nothing, and Selector::Parse
  // tells the two apart.
  [[nodiscard]] std::optional<DOMQuery> Select(const Selector& selector) const;
  [[nodiscard]] std::optional<DOMQuery> Select(std::string_view selector) const;
  [[nodiscard]] std::vector<DOMQuery> SelectAll(const Selector& selector) const;
  [[nodiscard]] std::vector<DOMQuery> SelectAll(std::string_view selector) const;

 private:
  // Calls visit on each descendant among the candidates (every descendant when nullopt)
  // that matches, in document order, until visit returns false.
  template <typename Matches, typename Visit>
  void forEachMatch(std::optional<NodeKeySpan> candidates, Matches&& matches, Visit&& visit) const;

  template <typename Visit>
  void forEachQueryMatch(const QueryOptions& options, Visit&& visit) const;

  [[nodiscard]] std::vector<NodeKey> selectKeys(const Selector& selector, bool first_only) const;

  // The shortest index list among the conditions that have one, or nullopt when none has
  // an index and every descendant is a candidate. An empty list settles the query.
  [[nodiscard]] std::optional<NodeKeySpan> indexedCandidates(const QueryOptions& options) const;
  [[nodiscard]] std::optional<NodeKeySpan> indexedCandidates(const CompoundSelector& compound) const;

  [[nodiscard]] bool matchAllConditions(const NodeRef& node, const QueryOptions& options) const;

  NodeRef subtree_root_;
  DOMSubtree subtree_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_QUERY_HPP_
