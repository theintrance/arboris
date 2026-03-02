/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_QUERY_HPP_
#define SRC_DOM_DOM_QUERY_HPP_

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/dom_builder.hpp"
#include "dom/dom_indexer.hpp"
#include "dom/dom_subtree.hpp"
#include "dom/tag_node.hpp"
#include "dom/html_token_parser.hpp"
#include "utils/string_pool.hpp"
#include "utils/query_options.hpp"

namespace arboris {

class DOMQuery {
 public:
  explicit DOMQuery(const DOMSubtree& subtree) :
    subtree_(subtree) {}

  DOMQuery(const DOMQuery&) = default;
  DOMQuery& operator=(const DOMQuery&) = delete;
  DOMQuery(DOMQuery&&) = default;
  DOMQuery& operator=(DOMQuery&&) = delete;
  virtual ~DOMQuery() = default;

  [[nodiscard]] const TagNode* operator->() const noexcept {
    return &getRoot();
  }

  std::optional<DOMQuery> Find(const QueryOptions& options) const;
  std::optional<DOMQuery> Find(const std::string& id) const;
  std::vector<DOMQuery> FindAll(const QueryOptions& options) const;

 private:
  const TagNode& getRoot() const noexcept {
    return subtree_.GetNodeByKey(subtree_.GetRootKey());
  }

  NodeKeySpan searchCandidatesFromSubtree(const QueryOptions& options) const;
  bool matchAllConditions(const TagNode& node, const QueryOptions& options) const;
  DOMQuery buildSubQuery(NodeKey subtree_root_key) const;

  DOMSubtree subtree_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_QUERY_HPP_
