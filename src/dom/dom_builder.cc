/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>
#include <utility>

#include "dom/dom_builder.hpp"
#include "dom/base_node.hpp"
#include "dom/tag_node.hpp"

namespace arboris {

bool DOMBuilder::Validate() const {
  return node_stack_.empty();
}

bool DOMBuilder::FeedOpenToken(HtmlToken&& token, const char* text_begin) {
  const bool is_void_tag = token.is_void_tag;
  const NodeKey parent_key = node_stack_.empty() ? root()->key() : node_stack_.top()->key();
  auto node = std::make_shared<TagNode>(next_node_key_++, parent_key, std::move(token));

  node_stack_.push(node);
  pending_marks_.push_back(static_cast<std::uint32_t>(pending_runs_.size()));

  node->set_text_content({text_begin, 0});  // NOTLINT(bugprone-string-constructor)

  dfs_node_list_.emplace_back(std::move(node));

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
void DOMBuilder::flushPendingRuns(const TagNodePtr& node) {
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
  const auto& root_node = root();
  flushPendingRuns(root_node);
  root_node->set_sub_tree_size(static_cast<std::uint32_t>(dfs_node_list_.size()));
}

bool DOMBuilder::FeedCloseToken(HtmlCloseToken&& token, const char* text_end) {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  auto top_node = node_stack_.top();
  if (token.tag != top_node->tag()) {
    return false;
  }

  auto text_begin = top_node->text_content().begin();
  top_node->set_text_content({text_begin, text_end});

  return closeTopNode();
}

bool DOMBuilder::closeTopNode() {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  if (node_stack_.empty()) {
    return false;
  }

  auto top_node = node_stack_.top();
  node_stack_.pop();

  flushPendingRuns(top_node);

  // Keys are handed out in DFS order, so everything created since this node opened is
  // inside it.
  top_node->set_sub_tree_size(next_node_key_ - top_node->key());

  return true;
}

}  // namespace arboris
