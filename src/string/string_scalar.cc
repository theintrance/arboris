/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "string/string.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>

namespace arboris {

void SkipWhitespace(std::string_view content, std::uint32_t& pos) {
  while (pos < content.length() && std::isspace(content[pos]))
    pos++;
}

std::string_view TrimWhitespace(std::string_view content) {
  std::uint32_t start = 0;
  std::uint32_t end = content.length();

  while (start < end && std::isspace(content[start]))
    start++;
  while (start < end && std::isspace(content[end - 1]))
    end--;

  return content.substr(start, end - start);
}

std::string ToLowercase(std::string_view content) {
  std::string result;
  result.reserve(content.length());

  for (const char c : content)
    result += std::tolower(c);

  return result;
}

bool IsCharAt(std::string_view content, std::uint32_t pos, char expected_char) {
  return pos < content.length() && content[pos] == expected_char;
}

void SkipWhitespace(std::string_view content, std::size_t& pos) {
  while (pos < content.length() && std::isspace(content[pos])) {
    ++pos;
  }
}

std::string_view ExtractSubstring(std::string_view content, std::size_t start, std::size_t end) {
  if (start >= content.length() || start >= end) {
    return std::string_view();
  }

  std::size_t actual_end = std::min(end, content.length());
  return content.substr(start, actual_end - start);
}

std::size_t FindNextChar(std::string_view content, std::size_t pos, char target_char) {
  while (pos < content.length()) {
    if (content[pos] == target_char) {
      return pos;
    }
    ++pos;
  }
  return content.length();
}

std::size_t FindNextAnyChar(std::string_view content, std::size_t pos, const char* target_chars) {
  while (pos < content.length()) {
    const char current_char = content[pos];
    for (const char* target = target_chars; *target != '\0'; ++target) {
      if (current_char == *target) {
        return pos;
      }
    }
    ++pos;
  }
  return content.length();
}

bool SkipUntilChar(std::string_view content, std::size_t& pos, char target_char) {
  while (pos < content.length()) {
    if (content[pos] == target_char) {
      return true;
    }
    ++pos;
  }
  return false;
}

bool IsAtEnd(std::string_view content, std::size_t pos) {
  return pos >= content.length();
}

}  // namespace arboris
