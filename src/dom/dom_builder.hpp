/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_BUILDER_HPP_
#define SRC_DOM_DOM_BUILDER_HPP_

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/tag.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// forward declaration
class Node;

class DOMBuilder {
 public:
  DOMBuilder() = default;
  DOMBuilder(const DOMBuilder&) = delete;
  DOMBuilder& operator=(const DOMBuilder&) = delete;
  DOMBuilder(DOMBuilder&&) = delete;
  DOMBuilder& operator=(DOMBuilder&&) = delete;
  virtual ~DOMBuilder() = default;

  bool Validate() const;
  bool FeedOpenToken(HtmlToken&& token);
  bool FeedTextToken(HtmlTextToken&& token);
  bool FeedCloseToken(HtmlCloseToken&& token);

 private:
  std::shared_ptr<Node> root_;
  std::stack<std::shared_ptr<Node>> node_stack_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_BUILDER_HPP_
