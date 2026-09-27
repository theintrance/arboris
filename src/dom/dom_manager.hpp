/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_MANAGER_HPP_
#define SRC_DOM_DOM_MANAGER_HPP_

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "dom/dom_query.hpp"
#include "dom/dom_store.hpp"
#include "dom/node_ref.hpp"
#include "utils/assertion.hpp"
#include "utils/query_options.hpp"
#include "utils/string_pool.hpp"

namespace arboris {

// Parses one document and owns everything that came out of it.
//
// The HTML is copied once into the store, and node ids, class names and attributes are
// views into that copy, so the caller's buffer does not have to outlive the DOM.
class DOMManager {
 public:
  explicit DOMManager(std::string_view html_content);
  DOMManager(const DOMManager&) = delete;
  DOMManager& operator=(const DOMManager&) = delete;
  DOMManager(DOMManager&&) = delete;
  DOMManager& operator=(DOMManager&&) = delete;
  virtual ~DOMManager() = default;

  [[nodiscard]] NodeRef GetRoot() const {
    ARBORIS_ASSERT(!store_.nodes().empty(), "Root node is nullptr.");
    return NodeRef(store_.nodes().front()->key(), store_);
  }

  [[nodiscard]] std::optional<DOMQuery> Find(const QueryOptions& options) const;
  [[nodiscard]] std::vector<DOMQuery> FindAll(const QueryOptions& options) const;

 private:
  DOMStore store_;
  std::shared_ptr<StringPool> string_pool_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_MANAGER_HPP_
