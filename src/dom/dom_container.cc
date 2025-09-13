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

void DOMContainer::feed_open_token(HtmlToken&& token) {
  // TODO(jayden): implement
}

void DOMContainer::feed_text_token(HtmlTextToken&& token) {
  // TODO(jayden): implement
}

void DOMContainer::feed_close_token(HtmlCloseToken&& token) {
  // TODO(jayden): implement
}

}  // namespace arboris
