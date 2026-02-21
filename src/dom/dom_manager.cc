/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>

#include "dom/dom_manager.hpp"

namespace arboris {

DOMManager::DOMManager(std::string_view html_content) :
  string_pool_(std::make_shared<StringPool>(html_content.size())),
  dom_indexer_() {
  DOMBuilder builder;
  HtmlTokenParser html_token_parser(html_content, string_pool_);

  html_token_parser.set_feed_open_token_callback(
      std::bind(&DOMBuilder::FeedOpenToken, &builder, std::placeholders::_1, std::placeholders::_2));

  html_token_parser.set_feed_text_token_callback(
      std::bind(&DOMBuilder::FeedTextToken, &builder, std::placeholders::_1));

  html_token_parser.set_feed_close_token_callback(
      std::bind(&DOMBuilder::FeedCloseToken, &builder, std::placeholders::_1, std::placeholders::_2));

  // Set up node creation callback for DOMBuilder to index nodes
  builder.SetNodeCreationCallback(
      std::bind(&DOMIndexer::AddNode, &dom_indexer_, std::placeholders::_1));

  html_token_parser.Parse();

  ARBORIS_ASSERT(builder.Validate(), "DOM structure is invalid after parsing.");
}



}  // namespace arboris
