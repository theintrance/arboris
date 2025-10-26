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
#include "dom/text_node.hpp"

namespace arboris {

bool DOMBuilder::Validate() const {
  return node_stack_.empty();
}

bool DOMBuilder::FeedOpenToken(HtmlToken&& token, const char* text_begin) {
  bool is_void_tag = token.is_void_tag;
  auto parent = node_stack_.empty() ? root_ : node_stack_.top();
  auto node = std::make_shared<TagNode>(
    next_node_id_++,
    std::move(token),
    parent);

  node->set_in(++euler_tour_timer_);
  node_stack_.push(node);
  if (parent) {
    parent->AddChild(node);
  }

  node->set_text_content({text_begin, 0});


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

  auto text_node = std::make_shared<TextNode>(
    next_node_id_++,
    token.text_content,
    top_node);

  top_node->AddChild(text_node);
  return true;
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
  top_node->set_out(++euler_tour_timer_);
  node_stack_.pop();
  return true;
}

}  // namespace arboris
