/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_container.hpp"

namespace arboris {

bool DOMContainer::validate() const {
  return node_stack_.empty();
}

bool DOMContainer::feed_open_token(HtmlToken&& token) {
  // TODO(jayden): implement
  return true;
}

bool DOMContainer::feed_text_token(HtmlTextToken&& token) {
  if (node_stack_.empty()) {
    return false;
  }

  // TODO(jayden): implement
  return true;
}

bool DOMContainer::feed_close_token(HtmlCloseToken&& token) {
  if (node_stack_.empty()) {
    return false;
  }

  // TODO(jayden): implement
  return true;
}

}  // namespace arboris
