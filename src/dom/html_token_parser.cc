/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/html_token_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "string/string.hpp"
#include "utils/string_pool.hpp"

namespace arboris {

namespace {

// `<!-- -->`, `<!DOCTYPE>` and `<?xml?>` open like a tag but are not one. The HTML tokenizer
// decides on the character after '<' alone: '!' opens a comment, a DOCTYPE, CDATA or else a
// bogus comment, and '?' always a bogus comment. So the first character is the whole rule;
// matching `<!--` or `<!DOCTYPE` in full would let bogus comments like `<![endif]>` through.
//
// Read as attributes, a comment's contents would hand the ids and classes of commented-out
// markup to the DOM, so until the parser reads these for what they are, they end at the
// first '>'.
bool IsCommentOrDeclaration(std::string_view tag_name) {
  return tag_name.front() == '!' || tag_name.front() == '?';
}

}  // anonymous namespace

bool HtmlTokenParser::Parse() const {
  std::size_t pos = 0;

  // One open tag's attributes at a time, so a tag costs no allocation once this has grown.
  std::vector<Attribute> attributes;

  while (pos < content_.length() && pos != std::string::npos) {
    if (content_[pos] != '<') {
      pos = parseTextContent(pos);
      continue;
    }

    if (pos + 1 < content_.length() && content_[pos + 1] == '/') {
      pos = parseCloseTag(pos);
      continue;
    }

    pos = parseOpenTag(pos, &attributes);
  }

  return pos != std::string::npos;
}

std::size_t HtmlTokenParser::parseOpenTag(std::size_t begin, std::vector<Attribute>* attributes) const {
  std::size_t current_pos = begin;
  ++current_pos;  // Skip '<'

  // Extract tag name
  std::string_view tag_name = extractTagName(&current_pos, kOpenTagDelimiters);
  if (tag_name.empty()) {
    return std::string::npos;
  }

  Tag tag = FromString(tag_name);

  // Find and skip '>', reading the attributes on the way
  attributes->clear();
  const bool closed =
      IsCommentOrDeclaration(tag_name) ? skipToTagEnd(&current_pos) : parseAttributes(&current_pos, attributes);
  if (!closed) {
    return std::string::npos;
  }

  // Create HtmlToken and call callback
  HtmlToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)}, tag, IsVoidTag(tag)};

  if (feed_open_token_callback_) {
    if (!feed_open_token_callback_(std::move(token), *attributes, string_pool_->GetCursor())) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::size_t HtmlTokenParser::parseCloseTag(std::size_t begin) const {
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
    if (!feed_close_token_callback_(std::move(token), string_pool_->GetCursor())) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::size_t HtmlTokenParser::parseTextContent(std::size_t begin) const {
  std::size_t current_pos = begin;

  // Read text until '<' or end of string
  current_pos = FindNextChar(content_, current_pos, '<');
  if (current_pos == std::string::npos) {
    current_pos = content_.length();
  }

  if (current_pos == begin) {
    return current_pos;
  }

  // Extract text content
  std::string_view text_content = ExtractSubstring(content_, begin, current_pos);
  auto pooled_text = string_pool_->Append(text_content);

  // Create HtmlTextToken and call callback
  HtmlTextToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)}, pooled_text};

  if (feed_text_token_callback_) {
    if (!feed_text_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::string_view HtmlTokenParser::extractTagName(std::size_t* begin, std::string_view delimiters) const {
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

// Reads the attributes between the tag name and the '>' that ends the tag, and leaves *begin
// just past that '>'. They have to be read to find it at all: a '>' inside a quoted value
// does not end the tag.
//
// Names and values point into the content. A name given twice keeps its first value, as the
// HTML tokenizer does. Returns false when the content ends before the tag does.
bool HtmlTokenParser::parseAttributes(std::size_t* begin, std::vector<Attribute>* attributes) const {
  std::size_t pos = SkipHtmlWhitespace(content_, *begin);

  while (pos < content_.length() && content_[pos] != '>') {
    // The '/' of `<br/>`, or a stray one: either way not part of an attribute.
    if (content_[pos] == '/') {
      pos = SkipHtmlWhitespace(content_, pos + 1);
      continue;
    }

    // The first character belongs to the name whatever it is, even '='.
    const std::size_t name_begin = pos;
    pos = FindNextAnyChar(content_, pos + 1, kAttributeNameDelimiters);
    if (pos == std::string::npos) {
      return false;
    }
    const std::string_view name = content_.substr(name_begin, pos - name_begin);

    // Without '=' the attribute has no value at all, and value keeps no data.
    std::string_view value;
    pos = SkipHtmlWhitespace(content_, pos);
    if (pos < content_.length() && content_[pos] == '=') {
      pos = SkipHtmlWhitespace(content_, pos + 1);
      if (pos >= content_.length()) {
        return false;
      }

      // substr rather than ExtractSubstring, which gives an empty range no data: an empty
      // value still has to point into the content, so has_value() tells `a=""` from `a`.
      const char quote = content_[pos];
      if (quote == '"' || quote == '\'') {
        const std::size_t value_end = FindNextChar(content_, pos + 1, quote);
        if (value_end == std::string::npos) {
          return false;
        }
        value = content_.substr(pos + 1, value_end - pos - 1);
        pos = value_end + 1;
      } else {
        const std::size_t value_end = FindNextAnyChar(content_, pos, kUnquotedValueDelimiters);
        if (value_end == std::string::npos) {
          return false;
        }
        value = content_.substr(pos, value_end - pos);
        pos = value_end;
      }
    }

    if (std::ranges::find(*attributes, name, &Attribute::name) == attributes->end()) {
      attributes->push_back({name, value});
    }
    pos = SkipHtmlWhitespace(content_, pos);
  }

  // The content ran out before the tag closed.
  if (pos >= content_.length()) {
    return false;
  }

  *begin = pos + 1;  // Skip '>'
  return true;
}

bool HtmlTokenParser::skipToTagEnd(std::size_t* begin) const {
  // Find '>'
  std::size_t found_pos = SkipUntilChar(content_, *begin, '>');
  if (found_pos == std::string::npos) {
    return false;
  }

  *begin = found_pos + 1;  // Skip '>'
  return true;
}

}  // namespace arboris
