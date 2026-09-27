/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>
#include <utility>

#include "dom/dom_builder.hpp"
#include "dom/tag_node.hpp"

namespace arboris {

bool DOMBuilder::Validate() const {
  return node_stack_.empty();
}

bool DOMBuilder::FeedOpenToken(HtmlToken&& token, const char* text_begin) {
  const bool is_void_tag = token.is_void_tag;
  const NodeKey parent_key = node_stack_.empty() ? root().key() : node_stack_.top();
  const NodeKey key = next_node_key_++;

  auto& node = dfs_node_list_.emplace_back(key, parent_key, std::move(token));
  node.set_text_content({text_begin, 0});  // NOLINT(bugprone-string-constructor)

  node_stack_.push(key);
  pending_marks_.push_back(static_cast<std::uint32_t>(pending_runs_.size()));

  if (is_void_tag) {
    return closeTopNode();
  }
  return true;
}

bool DOMBuilder::FeedTextToken(HtmlTextToken&& token) {
  pending_runs_.push_back(TextRun{token.text_content, token.begin_pos});
  return true;
}

// Moves the runs a node collected while it was open into the arena, side by side.
void DOMBuilder::flushPendingRuns(TagNode* node) {
  const auto mark = pending_marks_.empty() ? 0U : pending_marks_.back();
  if (!pending_marks_.empty()) {
    pending_marks_.pop_back();
  }

  const auto begin = static_cast<std::uint32_t>(text_arena_.size());
  text_arena_.insert(text_arena_.end(), pending_runs_.begin() + mark, pending_runs_.end());
  pending_runs_.resize(mark);

  node->set_text_runs(begin, static_cast<std::uint32_t>(text_arena_.size()) - begin);
}

void DOMBuilder::Finish() {
  // Parsing can stop early on markup the tokenizer cannot follow, leaving tags open. Their
  // subtree size would stay 0, and a walk that steps by subtree size would never move on.
  // Closing them here keeps every node's size at least 1 whatever the parse did.
  while (!node_stack_.empty()) {
    closeTopNode();
  }

  flushPendingRuns(&root());
  root().set_sub_tree_size(static_cast<std::uint32_t>(dfs_node_list_.size()));
}

bool DOMBuilder::FeedCloseToken(HtmlCloseToken&& token, const char* text_end) {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  TagNode& top_node = dfs_node_list_[node_stack_.top()];
  if (token.tag != top_node.tag()) {
    return false;
  }

  const auto* text_begin = top_node.text_content().data();
  top_node.set_text_content({text_begin, static_cast<std::size_t>(text_end - text_begin)});

  return closeTopNode();
}

bool DOMBuilder::closeTopNode() {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  if (node_stack_.empty()) {
    return false;
  }

  TagNode& top_node = dfs_node_list_[node_stack_.top()];
  node_stack_.pop();

  flushPendingRuns(&top_node);

  // Keys are handed out in DFS order, so everything created since this node opened is
  // inside it.
  top_node.set_sub_tree_size(next_node_key_ - top_node.key());

  return true;
}

}  // namespace arboris
