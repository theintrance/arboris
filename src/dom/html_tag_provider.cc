/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/html_tag_provider.hpp"

#include <cctype>
#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>

#include "string/string.hpp"

namespace arboris {

bool HtmlTagProvider::Parse() {
  std::size_t pos = 0;

  while (pos < content_.length()) {
    if (content_[pos] != '<') {
      if (!parseTextContent(&pos))
        return false;
      continue;
    }

    if (pos + 1 < content_.length() && content_[pos + 1] == '/') {
      if (!parseCloseTag(&pos))
        return false;
      continue;
    }

    if (!parseOpenTag(&pos))
      return false;
  }

  return true;
}

bool HtmlTagProvider::parseOpenTag(std::size_t* pos) {
  std::size_t start_pos = *pos;

  ++(*pos);  // Skip '<'

  // Extract tag name
  std::string_view tag_name = extractTagName(pos, kOpenTagDelimiters);
  if (tag_name.empty())
    return false;

  Tag tag = from_string(tag_name);

  // Find and skip '>'
  if (!skipToTagEnd(pos))
    return false;

  // Create HtmlToken and call callback
  HtmlToken token{{TokenType::kHtml, static_cast<std::uint32_t>(start_pos), static_cast<std::uint32_t>(*pos)},
                  tag,
                  is_void_tag(tag)};

  if (feed_open_token_callback_)
    return feed_open_token_callback_(std::move(token));

  return true;
}

bool HtmlTagProvider::parseCloseTag(std::size_t* pos) {
  std::size_t start_pos = *pos;

  *pos += 2;  // Skip '</'

  // Extract tag name
  std::string_view tag_name = extractTagName(pos, kCloseTagDelimiters);
  if (tag_name.empty())
    return false;

  Tag tag = from_string(tag_name);

  // Find and skip '>'
  if (!skipToTagEnd(pos))
    return false;

  // Create HtmlCloseToken and call callback
  HtmlCloseToken token{{TokenType::kHtml, static_cast<std::uint32_t>(start_pos), static_cast<std::uint32_t>(*pos)},
                       tag};

  if (feed_close_token_callback_)
    return feed_close_token_callback_(std::move(token));

  return true;
}

bool HtmlTagProvider::parseTextContent(std::size_t* pos) {
  std::size_t start_pos = *pos;

  // Read text until '<' or end of string
  *pos = FindNextChar(content_, *pos, '<');

  if (*pos == start_pos)
    return true;  // Empty text

  // Extract text content
  std::string_view text_content = ExtractSubstring(content_, start_pos, *pos);

  // Create HtmlTextToken and call callback
  HtmlTextToken token{{TokenType::kHtml, static_cast<std::uint32_t>(start_pos), static_cast<std::uint32_t>(*pos)},
                      text_content};

  if (feed_text_token_callback_)
    return feed_text_token_callback_(std::move(token));

  return true;
}

std::string_view HtmlTagProvider::extractTagName(std::size_t* pos, std::string_view delimiters) {
  // Find start of tag name
  SkipWhitespace(content_, *pos);
  std::size_t tag_name_start = *pos;

  // Find end of tag name (until specified delimiters)
  *pos = FindNextAnyChar(content_, *pos, delimiters.data());

  if (*pos == tag_name_start) {
    // Empty tag name
    return std::string_view();
  }

  std::string_view result = ExtractSubstring(content_, tag_name_start, *pos);
  return result;
}

bool HtmlTagProvider::skipToTagEnd(std::size_t* pos) {
  // Find '>'
  if (!SkipUntilChar(content_, *pos, '>'))
    return false;

  ++(*pos);  // Skip '>'
  return true;
}

}  // namespace arboris
