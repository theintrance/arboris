/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "parser/html_parser.hpp"

#include <cctype>
#include <cstdint>
#include <memory>
#include <stack>
#include <string_view>

#include "dom/dom_container.hpp"
#include "dom/node.hpp"
#include "string/string.hpp"
#include "utils/html_tokens.hpp"
#include "utils/tag.hpp"

namespace arboris {

HTMLParser::HTMLParser(std::string_view html_content)
    : dom_container_(std::make_shared<DOMContainer>()),
      html_content_(html_content) {}

std::shared_ptr<DOMContainer> HTMLParser::Parse() {
  initializeParsing();

  while (hasNextToken()) {
    skipWhitespace();
    
    if (current_pos_ >= html_content_.length())
      break;
    
    if (currentChar() == '<')
      parseAndProcessTag();
    else
      parseAndProcessText();
  }

  return dom_container_;
}

void HTMLParser::initializeParsing() {
  current_pos_ = 0;
  timer_ = 0;
  node_id_counter_ = 0;
  
  while (!node_stack_.empty())
    node_stack_.pop();
}

void HTMLParser::parseAndProcessTag() {
  std::uint32_t begin_pos = current_pos_;
  advance();
  
  // check if it is a close tag
  bool is_close_tag = false;
  if (currentChar() == '/') {
    is_close_tag = true;
    advance();
  }
  
  // parse tag name
  std::uint32_t tag_start = current_pos_;
  while (current_pos_ < html_content_.length() && 
         !std::isspace(currentChar()) && 
         currentChar() != '>' && 
         currentChar() != '/')
    advance();
  
  std::string_view tag_name = html_content_.substr(tag_start, current_pos_ - tag_start);
  Tag tag = parseTagName(tag_name);
  
  // TODO(team): parse attributes
  while (current_pos_ < html_content_.length() && currentChar() != '>')
    advance();

  // skip '>'
  if (current_pos_ < html_content_.length())
    advance();
  
  std::uint32_t end_pos = current_pos_;
  
  if (is_close_tag)
    processCloseToken(begin_pos, end_pos, tag);
  else
    processOpenToken(begin_pos, end_pos, tag);
}

void HTMLParser::parseAndProcessText() {
  std::uint32_t begin_pos = current_pos_;
  
  // parse text content
  while (current_pos_ < html_content_.length() && currentChar() != '<')
    advance();
  
  std::uint32_t end_pos = current_pos_;
  std::string_view text_content = html_content_.substr(begin_pos, end_pos - begin_pos);
  
  // remove whitespace
  text_content = trim_whitespace(text_content);
  
  if (!text_content.empty())
    processTextToken(begin_pos, end_pos, text_content);
}

void HTMLParser::processOpenToken(std::uint32_t begin_pos, std::uint32_t end_pos, Tag tag) {
  // TODO(team) : set parent node
  auto node = std::make_shared<Node>(
      ++node_id_counter_,
      HtmlToken{begin_pos, end_pos, tag, is_void_tag(tag)}
  );
  
  // set in time for euler path
  node->set_in(++timer_);

  // feed open token to DOM container
  HtmlToken feed_token;
  feed_token.begin_pos = begin_pos;
  feed_token.end_pos = end_pos;
  feed_token.tag = tag;
  feed_token.is_void_tag = is_void_tag(tag);
  dom_container_->feed_open_token(std::move(feed_token));

  // push node to stack
  node_stack_.push(node);
  
  // if it is a void tag, process it as a close tag
  if (is_void_tag(tag)) {
    node->set_out(timer_); // subtree completed
    node_stack_.pop();
  }
}

void HTMLParser::processCloseToken(std::uint32_t begin_pos, std::uint32_t end_pos, Tag tag) {
  if (node_stack_.empty())
    return;
    
  auto node = node_stack_.top();
  node_stack_.pop();
  
  node->set_out(timer_);
  
  HtmlCloseToken close_token;
  close_token.begin_pos = begin_pos;
  close_token.end_pos = end_pos;
  close_token.tag = tag;
  dom_container_->feed_close_token(std::move(close_token));
}

void HTMLParser::processTextToken(std::uint32_t begin_pos, std::uint32_t end_pos, std::string_view text_content) {
  if (node_stack_.empty())
    return;
    
  HtmlTextToken text_token;
  text_token.begin_pos = begin_pos;
  text_token.end_pos = end_pos;
  text_token.text_content = text_content;
  dom_container_->feed_text_token(std::move(text_token));
}

bool HTMLParser::hasNextToken() const {
  std::uint32_t pos = current_pos_;
  while (pos < html_content_.length() && std::isspace(html_content_[pos]))
    pos++;
  
  return pos < html_content_.length();
}


Tag HTMLParser::parseTagName(std::string_view tag_content) {
  // convert tag name to lowercase
  std::string lowercase_tag = to_lowercase(tag_content);
  return from_string(lowercase_tag);
}

void HTMLParser::skipWhitespace() {
  skip_whitespace(html_content_, current_pos_);
}

char HTMLParser::currentChar() const {
  if (current_pos_ >= html_content_.length())
    return '\0';
  return html_content_[current_pos_];
}

void HTMLParser::advance() {
  if (current_pos_ < html_content_.length())
    current_pos_++;
}

}  // namespace arboris
