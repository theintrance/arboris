/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_NODE_REF_HPP_
#define SRC_DOM_NODE_REF_HPP_

#include <span>
#include <string_view>

#include "dom/dom_store.hpp"
#include "dom/dom_types.hpp"
#include "dom/tag_node.hpp"
#include "utils/assertion.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// A handle to one node: a key and the store to resolve it against, 16 bytes, no ownership.
//
// Queries hand these out instead of the node itself, so the way nodes are stored stays
// behind this class. A key rather than a pointer, because the node array and the arenas
// grow while a document is parsed and every pointer into them moves with them.
class NodeRef {
 public:
  NodeRef(NodeKey key, const DOMStore& store) noexcept : key_(key), store_(&store) {}

  [[nodiscard]] NodeKey key() const noexcept {
    return key_;
  }

  [[nodiscard]] Tag tag() const noexcept {
    return node().tag();
  }

  [[nodiscard]] std::string_view id() const noexcept {
    return node().id();
  }

  [[nodiscard]] std::string_view text_content() const noexcept {
    return node().text_content();
  }

  [[nodiscard]] std::uint32_t sub_tree_size() const noexcept {
    return node().sub_tree_size();
  }

  [[nodiscard]] std::span<const std::string_view> classes() const noexcept {
    const auto& tag_node = node();
    return {store_->class_arena().data() + tag_node.class_begin(), tag_node.class_count()};
  }

  // The text written directly inside this tag, in document order. Text further down sits
  // on the node that holds it.
  [[nodiscard]] std::span<const TextRun> text_runs() const noexcept {
    const auto& tag_node = node();
    return {store_->text_arena().data() + tag_node.text_begin(), tag_node.text_count()};
  }

  [[nodiscard]] std::span<const Attribute> attributes() const noexcept {
    const auto& tag_node = node();
    return {store_->attr_arena().data() + tag_node.attr_begin(), tag_node.attr_count()};
  }

  [[nodiscard]] const TagNode& node() const noexcept {
    ARBORIS_ASSERT(key_ < store_->nodes().size(), "Node key is out of range.");
    return *store_->nodes()[key_];
  }

 private:
  NodeKey key_;
  const DOMStore* store_;
};

}  // namespace arboris

#endif  // SRC_DOM_NODE_REF_HPP_
