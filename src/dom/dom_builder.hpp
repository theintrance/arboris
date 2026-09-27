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
#include <utility>
#include <cstdint>
#include <vector>

#include "utils/html_tokens.hpp"
#include "dom/tag_node.hpp"

namespace arboris {

class DOMBuilder {
 public:
  DOMBuilder(): dfs_node_list_{std::make_shared<TagNode>(0, 0, HtmlToken{{0, 0}, Tag::kHtml, false})} {}
  DOMBuilder(const DOMBuilder&) = delete;
  DOMBuilder& operator=(const DOMBuilder&) = delete;
  DOMBuilder(DOMBuilder&&) = delete;
  DOMBuilder& operator=(DOMBuilder&&) = delete;
  virtual ~DOMBuilder() = default;

  [[nodiscard]] bool Validate() const;

  // Closes the root, which no end tag ever closes: its own text runs are flushed and its
  // subtree covers every node.
  void Finish();
  bool FeedOpenToken(HtmlToken&& token, const char* text_begin);
  bool FeedTextToken(HtmlTextToken&& token);
  bool FeedCloseToken(HtmlCloseToken&& token, const char* text_end);

  [[nodiscard]] const TagNodeList& GetNodeList() const {
    return dfs_node_list_;
  }

  // Hands the nodes over instead of copying every shared_ptr out of the builder.
  [[nodiscard]] TagNodeList ReleaseNodeList() {
    return std::move(dfs_node_list_);
  }

  [[nodiscard]] std::vector<TextRun> ReleaseTextArena() {
    return std::move(text_arena_);
  }

 private:
  [[nodiscard]] TagNodePtr root() {
    ARBORIS_ASSERT(!dfs_node_list_.empty(), "Root node is nullptr.");
    return dfs_node_list_.front();
  }

  bool closeTopNode();
  void flushPendingRuns(const TagNodePtr& node);

 private:
  NodeKey next_node_key_{1};

  TagNodeList dfs_node_list_;
  std::stack<TagNodePtr> node_stack_;

  // Text runs land here in document order, a node's own runs ending up side by side: a
  // node closes after every child it contains, so its pending runs are the tail left
  // above the mark it took when it opened.
  std::vector<TextRun> text_arena_;
  std::vector<TextRun> pending_runs_;
  std::vector<std::uint32_t> pending_marks_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_BUILDER_HPP_
