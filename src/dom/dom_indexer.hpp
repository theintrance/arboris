/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_INDEXER_HPP_
#define SRC_DOM_DOM_INDEXER_HPP_

#include <optional>
#include <string>
#include <unordered_map>

#include "dom/dom_types.hpp"
#include "dom/tag_node.hpp"

namespace arboris {

class DOMIndexer {
 public:
  DOMIndexer() = default;
  DOMIndexer(const DOMIndexer&) = delete;
  DOMIndexer& operator=(const DOMIndexer&) = delete;
  DOMIndexer(DOMIndexer&&) = delete;
  DOMIndexer& operator=(DOMIndexer&&) = delete;
  virtual ~DOMIndexer() = default;

  void AddNode(const NodePtr& node);

  [[nodiscard]] std::optional<NodeKey> GetNodeKeyById(std::string_view id) const;
  [[nodiscard]] std::optional<NodeKeyList> GetNodeKeyListByTag(Tag tag) const;
  [[nodiscard]] std::optional<NodeKeyList> GetNodeKeyListByClass(std::string_view class_name) const;
  [[nodiscard]] std::optional<NodeKeyList> GetNodeKeyListByAttribute(std::string_view attribute_name) const;

 private:
  // TODO(team): consider using std::list instead of std::vector for indexes
  std::unordered_map<std::string, NodeKey> id_index_;
  std::unordered_map<Tag, NodeKeyList> tag_index_;
  std::unordered_map<std::string, NodeKeyList> class_index_;

  // TODO(team): consider indexing by value instead of name
  std::unordered_map<std::string, NodeKeyList> attr_index_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_INDEXER_HPP_
