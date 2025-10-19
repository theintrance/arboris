/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>
#include <utility>

#include "dom/dom_builder.hpp"
#include "dom/node.hpp"

namespace arboris {

bool DOMBuilder::Validate() const {
  return node_stack_.empty();
}

bool DOMBuilder::FeedOpenToken(HtmlToken&& token) {
  bool is_void_tag = token.is_void_tag;
  auto parent = node_stack_.empty() ? nullptr : node_stack_.top();
  auto node = std::make_shared<Node>(
    next_node_id_++,

    // TODO(jayden): remove NOLINT after adding attributes in HtmlToken
    std::move(token),  // NOLINT(performance-move-const-arg)

    parent);

  node->set_in(++euler_tour_timer_);
  node_stack_.push(node);
  if (parent) {
    parent->AddChild(node);
  }

  if (!root_) {
    root_ = node;
  }

  // TODO(team): add attributes handling

  if (node_creation_callback_) {
    node_creation_callback_(node);
  }

  if (is_void_tag) {
    return closeTopNode();
  }
  return true;
}

bool DOMBuilder::FeedTextToken(HtmlTextToken&& token) {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  if (node_stack_.empty()) {
    return false;
  }

  auto top_node = node_stack_.top();

  /*
    TODO(jayden): Edge case handling for text token.

    Edge case example:
      <p>Hello <b>Beautiful</b> World!</p>
    
    "Hello" and "World!" should be added to the text content of the <p> tag.
    But this code currently cannot handle this edge case.
  */

  top_node->set_text_content(token.text_content);
  return true;
}

bool DOMBuilder::FeedCloseToken(HtmlCloseToken&& token) {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  auto top_node = node_stack_.top();
  if (token.tag != top_node->html_token().tag) {
    return false;
  }
  return closeTopNode();
}

bool DOMBuilder::closeTopNode() {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  if (node_stack_.empty()) {
    return false;
  }

  auto top_node = node_stack_.top();
  top_node->set_out(++euler_tour_timer_);
  node_stack_.pop();
  return true;
}

}  // namespace arboris
