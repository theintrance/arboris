/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "dom/html_token_parser.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "string/string.hpp"
#include "utils/string_pool.hpp"

namespace arboris {

namespace {

// <script> and <style> hold raw text, <textarea> and <title> RCDATA: either way their contents
// are text up to the element's own end tag, and a '<' in them is just a character. RCDATA
// also decodes character references, which nothing here does yet.
//
// The HTML standard switches to these states from the tree builder, and only for an HTML
// element: a <style> inside <svg> is ordinary markup. Deciding here comes to the same for
// contents without a '<', which is what an SVG <style> or <title> holds in practice.
constexpr TagSet kRawTextTags = CreateTagSet({Tag::kScript, Tag::kStyle, Tag::kTextarea, Tag::kTitle});

// What may follow the name in the end tag that closes a raw text element.
constexpr std::string_view kRawTextEndTagNameEnd = " \t\n\f\r/>";

constexpr std::string_view kCommentOpen = "<!--";
constexpr std::string_view kDoctypeOpen = "<!doctype";
constexpr std::string_view kDoctypeNameDelimiters = " \t\n\f\r>";

char ToAsciiLower(char c) {
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool EqualsIgnoringCase(std::string_view lhs, std::string_view rhs) {
  return std::ranges::equal(lhs, rhs, {}, ToAsciiLower, ToAsciiLower);
}

bool StartsWithIgnoringCase(std::string_view text, std::string_view prefix) {
  return text.size() >= prefix.size() && EqualsIgnoringCase(text.substr(0, prefix.size()), prefix);
}

}  // anonymous namespace

bool HtmlTokenParser::Parse() {
  std::size_t pos = 0;

  // One open tag's attributes at a time, so a tag costs no allocation once this has grown.
  std::vector<Attribute> attributes;

  // Set from a raw text element's start tag until its end tag is read.
  std::optional<RawTextElement> raw_text;

  while (pos < content_.length()) {
    const std::size_t token_begin = pos;
    const char next = pos + 1 < content_.length() ? content_[pos + 1] : '\0';

    if (raw_text) {
      // The contents, then the end tag that closes them, as two steps so that a failure is
      // reported at the one that failed. With no end tag to come, the text runs to the end
      // and the element is left open.
      const std::size_t text_end = findRawTextEnd(pos, raw_text->name);
      if (text_end > pos) {
        pos = feedText(pos, text_end);
      } else {
        // The end tag carries the element's own tag: its name was matched ignoring case,
        // so `</SCRIPT>` closes a <script> too.
        pos = feedCloseTag(pos, pos + 2 + raw_text->name.size(), raw_text->tag);
        raw_text.reset();
      }
    } else if (content_[pos] != '<') {
      pos = parseTextContent(pos);
    } else if (next == '/') {
      pos = parseCloseTag(pos);
    } else if (next == '!' || next == '?') {
      pos = parseMarkupDeclaration(pos);
    } else {
      pos = parseOpenTag(pos, &attributes, &raw_text);
    }

    if (pos == std::string::npos) {
      error_pos_ = token_begin;
      return false;
    }
  }

  return true;
}

std::size_t HtmlTokenParser::parseOpenTag(std::size_t begin, std::vector<Attribute>* attributes,
                                          std::optional<RawTextElement>* raw_text) const {
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
  bool self_closing = false;
  if (!parseAttributes(&current_pos, attributes, &self_closing)) {
    return std::string::npos;
  }

  // Create HtmlToken and call callback
  HtmlToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)},
                  tag, IsVoidTag(tag), self_closing};

  if (feed_open_token_callback_) {
    if (!feed_open_token_callback_(std::move(token), *attributes, string_pool_->GetCursor())) {
      return std::string::npos;
    }
  }

  // A self-closing <script/> or <title/> belongs to <svg>, which closes it on the spot. An
  // HTML one would still run to its end tag, but no well-formed page writes one.
  if (ContainsTag(kRawTextTags, tag) && !self_closing) {
    *raw_text = RawTextElement{.tag = tag, .name = tag_name};
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

  return feedCloseTag(begin, current_pos, FromString(tag_name));
}

// Finds and skips the '>' after an end tag's name, then hands the end tag over.
std::size_t HtmlTokenParser::feedCloseTag(std::size_t begin, std::size_t name_end, Tag tag) const {
  std::size_t current_pos = name_end;
  if (!skipToTagEnd(&current_pos)) {
    return std::string::npos;
  }

  HtmlCloseToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(current_pos)}, tag};

  if (feed_close_token_callback_) {
    if (!feed_close_token_callback_(std::move(token), string_pool_->GetCursor())) {
      return std::string::npos;
    }
  }

  return current_pos;
}

std::size_t HtmlTokenParser::parseTextContent(std::size_t begin) const {
  // Read text until '<' or end of string
  std::size_t end = FindNextChar(content_, begin, '<');
  if (end == std::string::npos) {
    end = content_.length();
  }

  return feedText(begin, end);
}

std::size_t HtmlTokenParser::feedText(std::size_t begin, std::size_t end) const {
  if (end == begin) {
    return end;
  }

  // Extract text content
  std::string_view text_content = ExtractSubstring(content_, begin, end);
  auto pooled_text = string_pool_->Append(text_content);

  // Create HtmlTextToken and call callback
  HtmlTextToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(end)}, pooled_text};

  if (feed_text_token_callback_) {
    if (!feed_text_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return end;
}

// Where the end tag that closes a raw text element starts: "</", the element's name in any
// case, then whitespace, '/' or '>'. Anything else, `</scripts>` or `</p>`, is text. The end
// of the content when no such tag follows.
std::size_t HtmlTokenParser::findRawTextEnd(std::size_t begin, std::string_view tag_name) const {
  for (std::size_t pos = content_.find("</", begin); pos != std::string_view::npos;
       pos = content_.find("</", pos + 1)) {
    const std::size_t name_end = pos + 2 + tag_name.size();
    if (name_end < content_.length() && EqualsIgnoringCase(content_.substr(pos + 2, tag_name.size()), tag_name) &&
        kRawTextEndTagNameEnd.find(content_[name_end]) != std::string_view::npos) {
      return pos;
    }
  }
  return content_.length();
}

// `<!` and `<?` open a comment, a DOCTYPE or a bogus comment, never an element. Read as a
// tag, a comment's contents would hand the ids and classes of commented-out markup to the
// DOM, and the comment would stay open as an element named `!--`.
//
// Inside <svg> and <math>, `<![CDATA[...]]>` is text; it is read as a bogus comment here,
// as it is in HTML.
std::size_t HtmlTokenParser::parseMarkupDeclaration(std::size_t begin) const {
  const std::string_view rest = content_.substr(begin);

  if (rest.starts_with(kCommentOpen)) {
    return parseComment(begin);
  }
  if (StartsWithIgnoringCase(rest, kDoctypeOpen)) {
    return parseDoctype(begin);
  }

  // `<?xml ...?>` keeps its '?' as data, `<!...>` drops its '!'.
  return parseBogusComment(begin, begin + (rest[1] == '?' ? 1 : 2));
}

// `<!--` up to the first `-->`. The HTML tokenizer also ends a comment at `--!>`, and reads
// `<!-->` and `<!--->` as empty ones. All three are errors, but reading them any other way
// would turn the page after them into comment. A comment left open runs to the end.
std::size_t HtmlTokenParser::parseComment(std::size_t begin) const {
  const std::size_t data_begin = begin + kCommentOpen.size();
  const std::string_view rest = content_.substr(data_begin);

  if (rest.starts_with(">")) {
    return feedComment(begin, data_begin + 1, rest.substr(0, 0));
  }
  if (rest.starts_with("->")) {
    return feedComment(begin, data_begin + 2, rest.substr(0, 0));
  }

  for (std::size_t dash = rest.find("--"); dash != std::string_view::npos; dash = rest.find("--", dash + 1)) {
    const std::string_view after = rest.substr(dash + 2);
    if (after.starts_with(">")) {
      return feedComment(begin, data_begin + dash + 3, rest.substr(0, dash));
    }
    if (after.starts_with("!>")) {
      return feedComment(begin, data_begin + dash + 4, rest.substr(0, dash));
    }
  }

  return feedComment(begin, content_.length(), rest);
}

// A bogus comment runs to the first '>', or to the end.
std::size_t HtmlTokenParser::parseBogusComment(std::size_t begin, std::size_t data_begin) const {
  const std::size_t data_end = std::min(FindNextChar(content_, data_begin, '>'), content_.length());
  const std::size_t end = data_end < content_.length() ? data_end + 1 : data_end;
  return feedComment(begin, end, content_.substr(data_begin, data_end - data_begin));
}

std::size_t HtmlTokenParser::feedComment(std::size_t begin, std::size_t end, std::string_view data) const {
  HtmlCommentToken token{{static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(end)}, data};

  if (feed_comment_token_callback_) {
    if (!feed_comment_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return end;
}

// `<!DOCTYPE html>`, or an older one with a public id, a system id or both. The keywords
// are read in any case. Anything else before the '>' makes it not well-formed, and it then
// ends at the first '>' outside an id.
std::size_t HtmlTokenParser::parseDoctype(std::size_t begin) const {
  HtmlDoctypeToken token;

  std::size_t pos = SkipHtmlWhitespace(content_, begin + kDoctypeOpen.size());
  const std::size_t name_end = std::min(FindNextAnyChar(content_, pos, kDoctypeNameDelimiters), content_.length());
  token.name = content_.substr(pos, name_end - pos);
  pos = SkipHtmlWhitespace(content_, name_end);

  bool well_formed = !token.name.empty();
  const std::string_view rest = content_.substr(pos);
  if (StartsWithIgnoringCase(rest, "public")) {
    pos = SkipHtmlWhitespace(content_, pos + 6);
    token.public_id = readDoctypeId(&pos, &well_formed);
    well_formed = well_formed && token.public_id.has_value();

    // The system id is optional after a public id.
    pos = SkipHtmlWhitespace(content_, pos);
    token.system_id = readDoctypeId(&pos, &well_formed);
  } else if (StartsWithIgnoringCase(rest, "system")) {
    pos = SkipHtmlWhitespace(content_, pos + 6);
    token.system_id = readDoctypeId(&pos, &well_formed);
    well_formed = well_formed && token.system_id.has_value();
  }
  pos = SkipHtmlWhitespace(content_, pos);

  const std::size_t close = FindNextChar(content_, pos, '>');
  const std::size_t end = close == std::string::npos ? content_.length() : close + 1;
  token.begin_pos = static_cast<std::uint32_t>(begin);
  token.end_pos = static_cast<std::uint32_t>(end);
  token.force_quirks = !well_formed || close != pos;

  if (feed_doctype_token_callback_) {
    if (!feed_doctype_token_callback_(std::move(token))) {
      return std::string::npos;
    }
  }

  return end;
}

// A quoted public or system id at *begin, or nullopt when no quote is there. A '>' before
// the closing quote ends the DOCTYPE, as it does in the HTML tokenizer, so a lost quote
// cannot run on into the page.
std::optional<std::string_view> HtmlTokenParser::readDoctypeId(std::size_t* begin, bool* well_formed) const {
  if (*begin >= content_.length() || (content_[*begin] != '"' && content_[*begin] != '\'')) {
    return std::nullopt;
  }

  const std::array<char, 2> delimiters{content_[*begin], '>'};
  const std::size_t id_begin = *begin + 1;
  const std::size_t id_end =
      std::min(FindNextAnyChar(content_, id_begin, {delimiters.data(), delimiters.size()}), content_.length());

  if (id_end == content_.length() || content_[id_end] == '>') {
    *well_formed = false;
    *begin = id_end;
  } else {
    *begin = id_end + 1;  // Skip the closing quote
  }
  return content_.substr(id_begin, id_end - id_begin);
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
// does not end the tag. *self_closing says whether the tag ended in "/>".
//
// Names and values point into the content. A name given twice keeps its first value, as the
// HTML tokenizer does. Returns false when the content ends before the tag does.
bool HtmlTokenParser::parseAttributes(std::size_t* begin, std::vector<Attribute>* attributes,
                                      bool* self_closing) const {
  std::size_t pos = SkipHtmlWhitespace(content_, *begin);

  while (pos < content_.length() && content_[pos] != '>') {
    // The '/' of `<br/>`, or a stray one: either way not part of an attribute. Only one
    // right before the '>' makes the tag self-closing.
    if (content_[pos] == '/') {
      *self_closing = pos + 1 < content_.length() && content_[pos + 1] == '>';
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
