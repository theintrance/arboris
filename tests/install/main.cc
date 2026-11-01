/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include <arboris/arboris.hpp>

// Exercises each public type once, as a user's first program would.
int main() {
  const auto dom = std::make_unique<arboris::DOMManager>("<ul id=l><li class=a>x<li>y &amp; z</ul>");

  const auto items = dom->SelectAll("ul > li");
  const auto first = dom->Find({.tag = arboris::Tag::kLi, .classes = arboris::ClassSet{"a"}});
  const auto list = dom->Find(std::string("l"));
  if (items.size() != 2 || !first || !list) {
    return 1;
  }

  const arboris::NodeRef second = items[1].Get();
  if (second.text_content() != "y & z" || second.outer_html() != "<li>y &amp; z" ||
      second.parent().id() != "l" || !second.previous_sibling() || second.ancestors().empty()) {
    return 2;
  }

  std::printf("%zu items, first: %.*s\n", items.size(), static_cast<int>(first->Get().text_content().size()),
              first->Get().text_content().data());
  return 0;
}
