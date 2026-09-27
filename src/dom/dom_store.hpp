/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_STORE_HPP_
#define SRC_DOM_DOM_STORE_HPP_

#include <string>
#include <string_view>
#include <utility>

#include "dom/dom_indexer.hpp"
#include "dom/dom_types.hpp"

namespace arboris {

// Everything one parsed document owns.
//
// The document buffer is the copy the DOM is built from, so the caller's HTML does not
// have to outlive the DOM. It is never touched again once parsing ends, which is what
// lets the nodes point into it rather than copy out of it.
class DOMStore {
 public:
  explicit DOMStore(std::string_view html_content) : document_(html_content) {}

  DOMStore(const DOMStore&) = delete;
  DOMStore& operator=(const DOMStore&) = delete;
  DOMStore(DOMStore&&) = delete;
  DOMStore& operator=(DOMStore&&) = delete;
  ~DOMStore() = default;

  [[nodiscard]] std::string_view document() const noexcept {
    return document_;
  }

  [[nodiscard]] const TagNodeList& nodes() const noexcept {
    return nodes_;
  }

  [[nodiscard]] const DOMIndexer& indexer() const noexcept {
    return indexer_;
  }

  void set_nodes(TagNodeList&& nodes) {
    nodes_ = std::move(nodes);
  }

  DOMIndexer& mutable_indexer() noexcept {
    return indexer_;
  }

 private:
  const std::string document_;

  TagNodeList nodes_;
  DOMIndexer indexer_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_STORE_HPP_
