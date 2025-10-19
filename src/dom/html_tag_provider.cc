/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/html_tag_provider.hpp"

#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include "string/string.hpp"

namespace arboris {

bool HtmlTagProvider::Parse() const {
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

std::size_t HtmlTagProvider::parseOpenTag(std::size_t begin) const {
  std::size_t current_pos = begin;
  ++current_pos;  // Skip '<'

  // Extract tag name
  std::string_view tag_name = extractTagName(&current_pos, kOpenTagDelimiters);
  if (tag_name.empty()) {
    return std::string::npos;
  }

  Tag tag = FromString(tag_name);

  // Find and skip '>'
  if (!skipToTagEnd(&current_pos)) {
    return std::string::npos;
  }

  // Create HtmlToken and call callback
  HtmlToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)}, tag, IsVoidTag(tag)};

  if (feed_open_token_callback_) {
    if (!feed_open_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::size_t HtmlTagProvider::parseCloseTag(std::size_t begin) const {
  std::size_t current_pos = begin;

  current_pos += 2;  // Skip '</'

  // Extract tag name
  std::string_view tag_name = extractTagName(&current_pos, kCloseTagDelimiters);
  if (tag_name.empty()) {
    return std::string::npos;
  }

  Tag tag = FromString(tag_name);

  // Find and skip '>'
  if (!skipToTagEnd(&current_pos)) {
    return std::string::npos;
  }

  // Create HtmlCloseToken and call callback
  HtmlCloseToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)}, tag};

  if (feed_close_token_callback_) {
    if (!feed_close_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::size_t HtmlTagProvider::parseTextContent(std::size_t begin) const {
  std::size_t current_pos = begin;

  // Read text until '<' or end of string
  current_pos = FindNextChar(content_, current_pos, '<');

  if (current_pos == begin) {
    return current_pos;
  }

  // Extract text content
  std::string_view text_content = ExtractSubstring(content_, begin, current_pos);

  // Create HtmlTextToken and call callback
  HtmlTextToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)}, text_content};

  if (feed_text_token_callback_) {
    if (!feed_text_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::string_view HtmlTagProvider::extractTagName(std::size_t* begin, std::string_view delimiters) const {
  // Find start of tag name
  *begin = SkipWhitespace(content_, *begin);
  std::size_t tag_name_start = *begin;

  // Find end of tag name (until specified delimiters)
  *begin = FindNextAnyChar(content_, *begin, delimiters);

  if (*begin == tag_name_start) {
    return std::string_view{};
  }

  std::string_view result = ExtractSubstring(content_, tag_name_start, *begin);
  return result;
}

bool HtmlTagProvider::skipToTagEnd(std::size_t* begin) const {
  // Find '>'
  std::size_t found_pos = SkipUntilChar(content_, *begin, '>');
  if (found_pos == std::string::npos) {
    return false;
  }

  *begin = found_pos + 1;  // Skip '>'
  return true;
}

}  // namespace arboris
