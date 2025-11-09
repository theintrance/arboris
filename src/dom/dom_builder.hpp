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
#include <functional>
#include <utility>
#include <cstdint>

#include "utils/html_tokens.hpp"
#include "dom/tag_node.hpp"

namespace arboris {

class DOMBuilder {
  using NodeCreationCallback = std::function<void(const std::shared_ptr<TagNode>&)>;

 public:
  DOMBuilder() : root_(std::make_shared<TagNode>(0, HtmlToken{{0, 0}, Tag::kHtml, false}, nullptr)) {}
  DOMBuilder(const DOMBuilder&) = delete;
  DOMBuilder& operator=(const DOMBuilder&) = delete;
  DOMBuilder(DOMBuilder&&) = delete;
  DOMBuilder& operator=(DOMBuilder&&) = delete;
  virtual ~DOMBuilder() = default;

  [[nodiscard]] bool Validate() const;
  bool FeedOpenToken(HtmlToken&& token, const char* text_begin);
  bool FeedTextToken(HtmlTextToken&& token);
  bool FeedCloseToken(HtmlCloseToken&& token, const char* text_end);

  void SetNodeCreationCallback(NodeCreationCallback&& callback) {
    node_creation_callback_ = std::move(callback);
  }

 private:
  bool closeTopNode();

 private:
  std::uint32_t next_node_id_{0};
  std::uint32_t euler_tour_timer_{0};

  std::shared_ptr<TagNode> root_;
  std::stack<std::shared_ptr<TagNode>> node_stack_;

  NodeCreationCallback node_creation_callback_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_BUILDER_HPP_
