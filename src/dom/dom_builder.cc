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
  return true;
}

bool DOMBuilder::FeedTextToken(HtmlTextToken&& token) {
  if (node_stack_.empty()) {
    return false;
  }

  // TODO(jayden): implement
  return true;
}

bool DOMBuilder::FeedCloseToken(HtmlCloseToken&& token) {
  if (node_stack_.empty()) {
    return false;
  }

  // TODO(jayden): implement
  return true;
}

}  // namespace arboris
