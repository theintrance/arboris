/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_INDEXER_HPP_
#define SRC_DOM_DOM_INDEXER_HPP_

#include <optional>
#include <string_view>
#include <unordered_map>

#include "dom/dom_types.hpp"
#include "utils/tag.hpp"

namespace arboris {

// Forward declaration: NodeRef resolves against the store that owns this indexer.
class NodeRef;

// Keys are string_views into the document buffer, which outlives the indexer, so indexing
// a class name and looking one up both stay free of allocation.
class DOMIndexer {
 public:
  DOMIndexer() = default;
  DOMIndexer(const DOMIndexer&) = delete;
  DOMIndexer& operator=(const DOMIndexer&) = delete;
  DOMIndexer(DOMIndexer&&) = delete;
  DOMIndexer& operator=(DOMIndexer&&) = delete;
  virtual ~DOMIndexer() = default;

  void AddNode(const NodeRef& node);

  [[nodiscard]] std::optional<NodeKey> GetNodeKeyById(std::string_view id) const;
  [[nodiscard]] NodeKeySpan GetNodeKeyListByTag(Tag tag) const;
  [[nodiscard]] NodeKeySpan GetNodeKeyListByClass(std::string_view class_name) const;
  [[nodiscard]] NodeKeySpan GetNodeKeyListByAttribute(std::string_view attribute_name) const;

 private:
  // TODO(team): consider using std::list instead of std::vector for indexes
  std::unordered_map<std::string_view, NodeKey> id_index_;
  std::unordered_map<Tag, NodeKeyList> tag_index_;
  std::unordered_map<std::string_view, NodeKeyList> class_index_;

  // TODO(team): consider indexing by value instead of name
  std::unordered_map<std::string_view, NodeKeyList> attr_index_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_INDEXER_HPP_
