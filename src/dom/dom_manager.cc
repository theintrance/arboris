/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>
#include <utility>
#include <vector>

#include "dom/dom_manager.hpp"
#include "dom/dom_types.hpp"

namespace arboris {

DOMManager::DOMManager(std::string_view html_content) :
  store_(html_content),
  string_pool_(std::make_shared<StringPool>(html_content.size())) {
  DOMBuilder builder;

  // Parsing reads the store's own copy of the document, not the caller's buffer.
  HtmlTokenParser html_token_parser(store_.document(), string_pool_);

  html_token_parser.set_feed_open_token_callback(
      std::bind(&DOMBuilder::FeedOpenToken, &builder, std::placeholders::_1, std::placeholders::_2));

  html_token_parser.set_feed_text_token_callback(
      std::bind(&DOMBuilder::FeedTextToken, &builder, std::placeholders::_1));

  html_token_parser.set_feed_close_token_callback(
      std::bind(&DOMBuilder::FeedCloseToken, &builder, std::placeholders::_1, std::placeholders::_2));

  // Set up node creation callback for DOMBuilder to index nodes
  builder.SetNodeCreationCallback(
      std::bind(&DOMIndexer::AddNode, &store_.mutable_indexer(), std::placeholders::_1));

  bool success = html_token_parser.Parse();
  ARBORIS_ASSERT(success, "Failed to parse HTML content.");

  TagNodeList nodes = builder.ReleaseNodeList();
  nodes.front()->set_sub_tree_size(static_cast<std::uint32_t>(nodes.size()));
  store_.set_nodes(std::move(nodes));

  ARBORIS_ASSERT(builder.Validate(), "DOM structure is invalid after parsing.");
}


std::optional<DOMQuery> DOMManager::Find(const QueryOptions& options) const {
  const auto& root = GetRoot();
  DOMSubtree root_subtree(store_, root);
  DOMQuery root_query(root, root_subtree);
  return root_query.Find(options);
}

std::vector<DOMQuery> DOMManager::FindAll(const QueryOptions& options) const {
  const auto& root = GetRoot();
  DOMSubtree root_subtree(store_, root);
  DOMQuery root_query(root, root_subtree);
  return root_query.FindAll(options);
}

}  // namespace arboris
