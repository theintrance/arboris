/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_ARBORIS_DOM_NODE_REF_HPP_
#define SRC_ARBORIS_DOM_NODE_REF_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <span>
#include <string_view>

#include "arboris/dom/dom_store.hpp"
#include "arboris/dom/dom_types.hpp"
#include "arboris/dom/tag_node.hpp"
#include "arboris/utils/assertion.hpp"
#include "arboris/utils/html_tokens.hpp"

namespace arboris {

template <typename Step>
class KeyRange;
struct ChildStep;
struct AncestorStep;
using ChildRange = KeyRange<ChildStep>;
using AncestorRange = KeyRange<AncestorStep>;

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

  // The tag name in lower case, "my-card" for a custom element; empty for the root.
  [[nodiscard]] std::string_view name() const noexcept {
    return node().name();
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

  // The element as written, start tag to end tag, cut from the document's UTF-8 copy: no
  // reassembly, so entities and spacing are the source's. For the root, the whole
  // document. An element the source never closed ends where whatever closed it begins.
  [[nodiscard]] std::string_view outer_html() const noexcept {
    const auto& tag_node = node();
    return {store_->document().data() + tag_node.source_begin(), tag_node.source_end() - tag_node.source_begin()};
  }

  // What stands between the start tag and the end tag, likewise.
  [[nodiscard]] std::string_view inner_html() const noexcept {
    const auto& tag_node = node();
    return {store_->document().data() + tag_node.content_begin(), tag_node.content_end() - tag_node.content_begin()};
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

  [[nodiscard]] NodeRef parent() const noexcept {
    return {node().parent_key(), *store_};
  }

  // The first element under this one, if any: the next key, since keys run in DFS order.
  [[nodiscard]] std::optional<NodeRef> first_child() const noexcept {
    if (node().sub_tree_size() <= 1) {
      return std::nullopt;
    }
    return NodeRef(key_ + 1, *store_);
  }

  [[nodiscard]] bool is_first_child() const noexcept {
    const auto& tag_node = node();
    return tag_node.parent_key() != key_ && tag_node.parent_key() + 1 == key_;
  }

  // The element after this one under the same parent, if any. The root has no siblings.
  [[nodiscard]] std::optional<NodeRef> next_sibling() const noexcept {
    const auto& tag_node = node();
    if (tag_node.parent_key() == key_) {
      return std::nullopt;
    }
    const auto& parent_node = store_->nodes()[tag_node.parent_key()];
    const NodeKey next = key_ + std::max(1U, tag_node.sub_tree_size());
    if (next >= parent_node.key() + parent_node.sub_tree_size()) {
      return std::nullopt;
    }
    return NodeRef(next, *store_);
  }

  // The element before this one under the same parent, if any. In DFS order the key just
  // before this one is that sibling or its last descendant, so it is reached by climbing
  // from there until the parent is shared: O(depth), not O(siblings).
  [[nodiscard]] std::optional<NodeRef> previous_sibling() const noexcept {
    const NodeKey parent_key = node().parent_key();
    if (parent_key == key_ || parent_key + 1 == key_) {
      return std::nullopt;
    }
    NodeKey candidate = key_ - 1;
    while (store_->nodes()[candidate].parent_key() != parent_key) {
      candidate = store_->nodes()[candidate].parent_key();
    }
    return NodeRef(candidate, *store_);
  }

  // The element children, in document order. A range, not a list: the first child is the
  // next key, and each sibling sits one whole subtree further on.
  [[nodiscard]] ChildRange children() const noexcept;

  // The parent, its parent and so on, up to but not including the synthetic root.
  [[nodiscard]] AncestorRange ancestors() const noexcept;

  [[nodiscard]] const TagNode& node() const noexcept {
    ARBORIS_ASSERT(key_ < store_->nodes().size(), "Node key is out of range.");
    return store_->nodes()[key_];
  }

 private:
  NodeKey key_;
  const DOMStore* store_;
};

// A walk over keys is a step policy: the next key, and whether a key is at or past the end.
struct ChildStep {
  // Each sibling sits one whole subtree further on. Never step by nothing: a size of 0
  // would stand here forever.
  [[nodiscard]] static NodeKey Next(NodeKey key, const DOMStore& store) noexcept {
    return key + std::max(1U, store.nodes()[key].sub_tree_size());
  }

  // An ordering, not an equality: a step over a subtree Finish never closed can land past
  // the end, and must not run the walk off the node array.
  [[nodiscard]] static bool Done(NodeKey key, NodeKey end) noexcept {
    return key >= end;
  }
};

struct AncestorStep {
  [[nodiscard]] static NodeKey Next(NodeKey key, const DOMStore& store) noexcept {
    return store.nodes()[key].parent_key();
  }

  // Parent keys descend to the root, key 0, which is its own parent.
  [[nodiscard]] static bool Done(NodeKey key, NodeKey end) noexcept {
    return key == end;
  }
};

// Hands out a NodeRef by value, so it is an input iterator to pre-C++20 algorithms and a
// forward iterator to ranges. A key Step says is done is stored as the end key, so
// comparing equal to end() is the stop condition however far a step overshot.
template <typename Step>
class KeyIterator {
 public:
  using iterator_concept = std::forward_iterator_tag;
  using iterator_category = std::input_iterator_tag;
  using value_type = NodeRef;
  using difference_type = std::ptrdiff_t;

  // What operator-> points into: there is no NodeRef stored anywhere to point at.
  class Pointer {
   public:
    explicit Pointer(NodeRef ref) noexcept : ref_(ref) {}

    [[nodiscard]] const NodeRef* operator->() const noexcept {
      return &ref_;
    }

   private:
    NodeRef ref_;
  };

  KeyIterator() noexcept = default;
  KeyIterator(NodeKey key, NodeKey end, const DOMStore& store) noexcept
      : key_(Step::Done(key, end) ? end : key), end_(end), store_(&store) {}

  [[nodiscard]] NodeRef operator*() const noexcept {
    return {key_, *store_};
  }

  [[nodiscard]] Pointer operator->() const noexcept {
    return Pointer{**this};
  }

  KeyIterator& operator++() noexcept {
    const NodeKey next = Step::Next(key_, *store_);
    key_ = Step::Done(next, end_) ? end_ : next;
    return *this;
  }

  KeyIterator operator++(int) noexcept {
    const KeyIterator before = *this;
    ++*this;
    return before;
  }

  [[nodiscard]] bool operator==(const KeyIterator& other) const noexcept {
    return key_ == other.key_;
  }

 private:
  NodeKey key_ = 0;
  NodeKey end_ = 0;
  const DOMStore* store_ = nullptr;
};

template <typename Step>
class KeyRange {
 public:
  using iterator = KeyIterator<Step>;

  KeyRange(NodeKey first, NodeKey end, const DOMStore& store) noexcept : first_(first), end_(end), store_(&store) {}

  [[nodiscard]] iterator begin() const noexcept {
    return {first_, end_, *store_};
  }

  [[nodiscard]] iterator end() const noexcept {
    return {end_, end_, *store_};
  }

  [[nodiscard]] bool empty() const noexcept {
    return Step::Done(first_, end_);
  }

 private:
  NodeKey first_;
  NodeKey end_;
  const DOMStore* store_;
};

inline ChildRange NodeRef::children() const noexcept {
  return {key_ + 1, key_ + node().sub_tree_size(), *store_};
}

// The end is the synthetic root, key 0, which every walk up reaches and none hands out.
inline AncestorRange NodeRef::ancestors() const noexcept {
  return {node().parent_key(), 0, *store_};
}

}  // namespace arboris

#endif  // SRC_ARBORIS_DOM_NODE_REF_HPP_
