/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_PARSER_HTML_PARSER_HPP_
#define SRC_PARSER_HTML_PARSER_HPP_

#include <cstdint>
#include <memory>
#include <stack>
#include <string_view>
#include <vector>

#include "dom/dom_container.hpp"
#include "dom/node.hpp"
#include "utils/html_tokens.hpp"
#include "utils/tag.hpp"

namespace arboris {

class HTMLParser {
 public:
  explicit HTMLParser(std::string_view html_content);
  
  HTMLParser(const HTMLParser&) = delete;
  HTMLParser& operator=(const HTMLParser&) = delete;
  HTMLParser(HTMLParser&&) = delete;
  HTMLParser& operator=(HTMLParser&&) = delete;
  
  virtual ~HTMLParser() = default;

  std::shared_ptr<DOMContainer> parse();

 private:
  void InitializeParsing();
  
  void ParseAndProcessTag();
  void ProcessOpenToken(std::uint32_t begin_pos, std::uint32_t end_pos, Tag tag);
  void ProcessCloseToken(std::uint32_t begin_pos, std::uint32_t end_pos, Tag tag);

  void ParseAndProcessText();
  void ProcessTextToken(std::uint32_t begin_pos, std::uint32_t end_pos, std::string_view text_content);

  Tag ParseTagName(std::string_view tag_content);
  
  void SkipWhitespace();
  void Advance();
  
  bool is_current_char(char c) const;
  char current_char() const;
  bool has_next_token() const;

  std::shared_ptr<DOMContainer> dom_container_;
  std::string_view html_content_;
  
  std::stack<std::shared_ptr<Node>> node_stack_;
  std::uint32_t timer_{0};
  std::uint32_t node_id_counter_{0};
  std::uint32_t current_pos_{0};
};

}  // namespace arboris

#endif  // SRC_PARSER_HTML_PARSER_HPP_
