/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_INDEXER_HPP_
#define SRC_DOM_DOM_INDEXER_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

  void AddNode(const std::shared_ptr<TagNode>& node);

 private:
  // TODO(team): consider using std::list instead of std::vector for indexes
  std::unordered_map<std::string, std::shared_ptr<TagNode>> id_index_;
  std::unordered_map<Tag, std::vector<std::shared_ptr<TagNode>>> tag_index_;
  std::unordered_map<std::string, std::vector<std::shared_ptr<TagNode>>> class_index_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_INDEXER_HPP_
