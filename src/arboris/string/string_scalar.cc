/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

#include "arboris/string/string.hpp"

namespace arboris {

std::string AsciiLowerCase(std::string_view text) {
  std::string out(text);
  std::ranges::transform(out, out.begin(), ToAsciiLower);
  return out;
}

bool EqualsIgnoringCase(std::string_view lhs, std::string_view rhs) {
  return std::ranges::equal(lhs, rhs, {}, ToAsciiLower, ToAsciiLower);
}

bool StartsWithIgnoringCase(std::string_view text, std::string_view prefix) {
  return text.size() >= prefix.size() && EqualsIgnoringCase(text.substr(0, prefix.size()), prefix);
}

std::size_t FindIgnoringCase(std::string_view haystack, std::string_view needle, std::size_t from) {
  for (std::size_t pos = from; pos + needle.size() <= haystack.size(); ++pos) {
    if (EqualsIgnoringCase(haystack.substr(pos, needle.size()), needle)) {
      return pos;
    }
  }
  return std::string_view::npos;
}

std::size_t FindNextOf(std::string_view content, std::size_t begin, const CharSet& set) {
  for (std::size_t i = begin; i < content.length(); ++i) {
    if (set.Contains(content[i])) {
      return i;
    }
  }
  return std::string::npos;
}

std::size_t SkipAll(std::string_view content, std::size_t begin, const CharSet& set) {
  std::size_t i = begin;
  while (i < content.length() && set.Contains(content[i])) {
    ++i;
  }
  return i;
}

std::size_t SkipWhitespace(std::string_view content, std::size_t begin) {
  while (begin < content.length() && std::isspace(static_cast<unsigned char>(content[begin])) != 0) {
    ++begin;
  }
  return begin;
}

std::size_t SkipHtmlWhitespace(std::string_view content, std::size_t begin) {
  return SkipAll(content, begin, kHtmlWhitespaceSet);
}

std::string_view ExtractSubstring(std::string_view content, std::size_t start, std::size_t end) {
  if (start >= content.length() || start >= end) {
    return std::string_view{};
  }
  return content.substr(start, std::min(end, content.length()) - start);
}

std::size_t FindNextChar(std::string_view content, std::size_t begin, char target_char) {
  if (begin >= content.length()) {
    return std::string::npos;
  }
  const void* found = std::memchr(content.data() + begin, target_char, content.length() - begin);
  return found == nullptr ? std::string::npos
                          : static_cast<std::size_t>(static_cast<const char*>(found) - content.data());
}

std::size_t FindNextAnyChar(std::string_view content, std::size_t begin, std::string_view target_chars) {
  return FindNextOf(content, begin, CharSet(target_chars));
}

std::size_t SkipUntilChar(std::string_view content, std::size_t begin, char target_char) {
  return FindNextChar(content, begin, target_char);
}

}  // namespace arboris
