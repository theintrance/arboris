/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_MANAGER_HPP_
#define SRC_DOM_DOM_MANAGER_HPP_

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/dom_builder.hpp"
#include "dom/dom_indexer.hpp"
#include "dom/dom_query.hpp"
#include "dom/html_token_parser.hpp"
#include "utils/string_pool.hpp"
#include "utils/query_options.hpp"

namespace arboris {

class DOMManager {
 public:
  explicit DOMManager(std::string_view html_content);
  DOMManager(const DOMManager&) = delete;
  DOMManager& operator=(const DOMManager&) = delete;
  DOMManager(DOMManager&&) = delete;
  DOMManager& operator=(DOMManager&&) = delete;
  virtual ~DOMManager() = default;

  [[nodiscard]] const TagNode& GetRoot() const {
    ARBORIS_ASSERT(!dfs_node_list_.empty(), "Root node is nullptr.");
    return *dfs_node_list_.front();
  }

  std::optional<DOMQuery> Find(const QueryOptions& options) const;
  std::vector<DOMQuery> FindAll(const QueryOptions& options) const;

 private:
  // Nodes list ordered by DFS in-order
  NodeList dfs_node_list_;
  std::shared_ptr<StringPool> string_pool_;
  DOMIndexer dom_indexer_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_MANAGER_HPP_
