/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_QUERY_HPP_
#define SRC_DOM_DOM_QUERY_HPP_

#include <optional>
#include <string>
#include <vector>

#include "dom/dom_subtree.hpp"
#include "dom/dom_types.hpp"
#include "dom/node_ref.hpp"
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

 private:
  [[nodiscard]] NodeKeySpan searchCandidatesFromSubtree(const QueryOptions& options) const;
  [[nodiscard]] bool matchAllConditions(const NodeRef& node, const QueryOptions& options) const;

  NodeRef subtree_root_;
  DOMSubtree subtree_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_QUERY_HPP_
