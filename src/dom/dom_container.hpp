/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_CONTAINER_HPP_
#define SRC_DOM_DOM_CONTAINER_HPP_

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

class DOMContainer {
 public:
  DOMContainer() = default;
  DOMContainer(const DOMContainer&) = delete;
  DOMContainer& operator=(const DOMContainer&) = delete;
  DOMContainer(DOMContainer&&) = delete;
  DOMContainer& operator=(DOMContainer&&) = delete;
  virtual ~DOMContainer() = default;

  bool validate() const;
  bool feed_open_token(HtmlToken&& token);
  bool feed_text_token(HtmlTextToken&& token);
  bool feed_close_token(HtmlCloseToken&& token);

 private:
  std::shared_ptr<Node> root_;
  std::stack<std::shared_ptr<Node>> node_stack_;

  // TODO(team): consider using std::list instead of std::vector for indexes
  std::unordered_map<std::string, std::shared_ptr<Node>> id_index_;
  std::unordered_map<Tag, std::vector<std::shared_ptr<Node>>> tag_index_;
  std::unordered_map<std::string, std::vector<std::shared_ptr<Node>>> class_index_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_CONTAINER_HPP_
