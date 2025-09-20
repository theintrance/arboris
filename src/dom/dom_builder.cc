/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_builder.hpp"

namespace arboris {

bool DOMBuilder::Validate() const {
  return node_stack_.empty();
}

bool DOMBuilder::FeedOpenToken(HtmlToken&& token) {
  // TODO(jayden): implement
  // TODO(jayden): call node_creation_callback_
  return true;
}

bool DOMBuilder::FeedTextToken(HtmlTextToken&& token) {
  // TODO(jayden): implement proper text token handling
  // For now, accept text tokens even if stack is empty for testing purposes
  return true;
}

bool DOMBuilder::FeedCloseToken(HtmlCloseToken&& token) {
  // TODO(jayden): implement proper close token handling
  // For now, accept close tokens even if stack is empty for testing purposes
  return true;
}

}  // namespace arboris
