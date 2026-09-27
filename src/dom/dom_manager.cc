/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/dom_manager.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "dom/dom_builder.hpp"
#include "dom/dom_subtree.hpp"
#include "dom/dom_types.hpp"
#include "dom/html_token_parser.hpp"

namespace arboris {

DOMManager::DOMManager(std::string_view html_content) :
  store_(html_content),
  string_pool_(std::make_shared<StringPool>(html_content.size())) {
  DOMBuilder builder;

  // Parsing reads the store's own copy, so every view a token keeps points into a buffer
  // that outlives the caller's.
  HtmlTokenParser html_token_parser(store_.document(), string_pool_);

  html_token_parser.set_feed_open_token_callback(
      [&builder](HtmlToken&& token, const char* text_begin) {
        return builder.FeedOpenToken(std::move(token), text_begin);
      });

  html_token_parser.set_feed_text_token_callback(
      [&builder](HtmlTextToken&& token) { return builder.FeedTextToken(std::move(token)); });

  html_token_parser.set_feed_close_token_callback(
      [&builder](HtmlCloseToken&& token, const char* text_end) {
        return builder.FeedCloseToken(std::move(token), text_end);
      });

  const bool success = html_token_parser.Parse();
  ARBORIS_ASSERT(success, "Failed to parse HTML content.");

  builder.Finish();
  store_.set_nodes(builder.ReleaseNodeList());
  store_.set_text_arena(builder.ReleaseTextArena());

  // Indexing walks the finished nodes in key order, which is also memory order.
  for (const auto& node : store_.nodes()) {
    store_.mutable_indexer().AddNode(NodeRef(node->key(), store_));
  }

  ARBORIS_ASSERT(builder.Validate(), "DOM structure is invalid after parsing.");
}

std::optional<DOMQuery> DOMManager::Find(const QueryOptions& options) const {
  const auto root = GetRoot();
  DOMSubtree root_subtree(store_, root.node());
  return DOMQuery(root, root_subtree).Find(options);
}

std::vector<DOMQuery> DOMManager::FindAll(const QueryOptions& options) const {
  const auto root = GetRoot();
  DOMSubtree root_subtree(store_, root.node());
  return DOMQuery(root, root_subtree).FindAll(options);
}

}  // namespace arboris
