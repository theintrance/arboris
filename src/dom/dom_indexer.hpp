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

  [[nodiscard]] NodePtr GetNodeById(std::string_view id) const;
  [[nodiscard]] std::optional<NodeList> GetNodesByTag(Tag tag) const;
  [[nodiscard]] std::optional<NodeList> GetNodesByClass(std::string_view class_name) const;

 private:
  // TODO(team): consider using std::list instead of std::vector for indexes
  std::unordered_map<std::string, NodePtr> id_index_;
  std::unordered_map<Tag, NodeList> tag_index_;
  std::unordered_map<std::string, NodeList> class_index_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_INDEXER_HPP_
