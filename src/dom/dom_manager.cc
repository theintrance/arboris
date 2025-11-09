/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <memory>

#include "dom/dom_manager.hpp"

namespace arboris {

DOMManager::DOMManager(std::string_view html_content) {
  string_pool_ = std::make_shared<StringPool>(html_content.size());
  dom_builder_ = std::make_unique<DOMBuilder>();
  dom_indexer_ = std::make_unique<DOMIndexer>();
  html_token_parser_ = std::make_unique<HtmlTokenParser>(html_content, string_pool_);

  parse();
}

void DOMManager::parse() {
  // Set up callbacks for HtmlTokenParser
  html_token_parser_->set_feed_open_token_callback(
      std::bind(&DOMBuilder::FeedOpenToken, dom_builder_.get(), std::placeholders::_1, std::placeholders::_2));

  html_token_parser_->set_feed_text_token_callback(
      std::bind(&DOMBuilder::FeedTextToken, dom_builder_.get(), std::placeholders::_1));

  html_token_parser_->set_feed_close_token_callback(
      std::bind(&DOMBuilder::FeedCloseToken, dom_builder_.get(), std::placeholders::_1, std::placeholders::_2));

  // Set up node creation callback for DOMBuilder to index nodes
  dom_builder_->SetNodeCreationCallback(
      std::bind(&DOMIndexer::AddNode, dom_indexer_.get(), std::placeholders::_1));

  // Start parsing
  html_token_parser_->Parse();
}

}  // namespace arboris
