/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/html_tag_provider.hpp"

#include <cctype>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "string/string.hpp"

namespace arboris {

bool HtmlTagProvider::Parse() {
  std::size_t pos = 0;

  while (pos < content_.length() && pos != std::string::npos) {
    if (content_[pos] != '<') {
      pos = parseTextContent(pos);
      continue;
    }

    if (pos + 1 < content_.length() && content_[pos + 1] == '/') {
      pos = parseCloseTag(pos);
      continue;
    }

    pos = parseOpenTag(pos);
  }

  return pos != std::string::npos;
}

std::size_t HtmlTagProvider::parseOpenTag(std::size_t begin) {
  std::size_t start_pos = begin;

  ++begin;  // Skip '<'

  // Extract tag name
  std::string_view tag_name = extractTagName(&begin, kOpenTagDelimiters);
  if (tag_name.empty())
    return std::string::npos;

  Tag tag = from_string(tag_name);

  // Find and skip '>'
  if (!skipToTagEnd(&begin))
    return std::string::npos;

  // Create HtmlToken and call callback
  HtmlToken token{{static_cast<std::uint32_t>(start_pos), static_cast<std::uint32_t>(begin)}, tag, is_void_tag(tag)};

  if (feed_open_token_callback_) {
    if (!feed_open_token_callback_(std::move(token)))
      return std::string::npos;
  }

  return begin;
}

std::size_t HtmlTagProvider::parseCloseTag(std::size_t begin) {
  std::size_t start_pos = begin;

  begin += 2;  // Skip '</'

  // Extract tag name
  std::string_view tag_name = extractTagName(&begin, kCloseTagDelimiters);
  if (tag_name.empty())
    return std::string::npos;

  Tag tag = from_string(tag_name);

  // Find and skip '>'
  if (!skipToTagEnd(&begin))
    return std::string::npos;

  // Create HtmlCloseToken and call callback
  HtmlCloseToken token{{static_cast<std::uint32_t>(start_pos), static_cast<std::uint32_t>(begin)}, tag};

  if (feed_close_token_callback_) {
    if (!feed_close_token_callback_(std::move(token)))
      return std::string::npos;
  }

  return begin;
}

std::size_t HtmlTagProvider::parseTextContent(std::size_t begin) {
  std::size_t start_pos = begin;

  // Read text until '<' or end of string
  begin = FindNextChar(content_, begin, '<');

  if (begin == start_pos)
    return begin;  // Empty text, return current position

  // Extract text content
  std::string_view text_content = ExtractSubstring(content_, start_pos, begin);

  // Create HtmlTextToken and call callback
  HtmlTextToken token{{static_cast<std::uint32_t>(start_pos), static_cast<std::uint32_t>(begin)}, text_content};

  if (feed_text_token_callback_) {
    if (!feed_text_token_callback_(std::move(token)))
      return std::string::npos;
  }

  return begin;
}

std::string_view HtmlTagProvider::extractTagName(std::size_t* begin, std::string_view delimiters) {
  // Find start of tag name
  *begin = SkipWhitespace(content_, *begin);
  std::size_t tag_name_start = *begin;

  // Find end of tag name (until specified delimiters)
  *begin = FindNextAnyChar(content_, *begin, delimiters.data());

  if (*begin == tag_name_start)
    return std::string_view();

  std::string_view result = ExtractSubstring(content_, tag_name_start, *begin);
  return result;
}

bool HtmlTagProvider::skipToTagEnd(std::size_t* begin) {
  // Find '>'
  std::size_t found_pos = SkipUntilChar(content_, *begin, '>');
  if (found_pos == std::string::npos)
    return false;

  *begin = found_pos + 1;  // Skip '>'
  return true;
}

}  // namespace arboris
