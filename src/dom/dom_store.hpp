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
#include <vector>

#include "dom/dom_indexer.hpp"
#include "dom/dom_types.hpp"
#include "dom/tag_node.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// Everything one parsed document owns.
//
// The document buffer is the single source of truth: node ids, class names and attribute
// names and values are all string_views into it, so nothing copies the bytes that are
// already there. That only holds while this store is alive and the buffer is never
// touched again after parsing, so the buffer is private and handed out by view alone.
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

  [[nodiscard]] const std::vector<std::string_view>& class_arena() const noexcept {
    return class_arena_;
  }

  [[nodiscard]] const std::vector<Attribute>& attr_arena() const noexcept {
    return attr_arena_;
  }

  [[nodiscard]] const std::vector<TextRun>& text_arena() const noexcept {
    return text_arena_;
  }

  void set_nodes(TagNodeList&& nodes) {
    nodes_ = std::move(nodes);
  }

  void set_text_arena(std::vector<TextRun>&& text_arena) {
    text_arena_ = std::move(text_arena);
  }

  DOMIndexer& mutable_indexer() noexcept {
    return indexer_;
  }

 private:
  const std::string document_;

  TagNodeList nodes_;

  // A node's classes and attributes live here, its own run inside the document's one
  // array. The node keeps the run's offset and length; see HtmlToken.
  std::vector<std::string_view> class_arena_;
  std::vector<Attribute> attr_arena_;
  std::vector<TextRun> text_arena_;

  DOMIndexer indexer_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_STORE_HPP_
