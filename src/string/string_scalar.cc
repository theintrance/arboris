/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include "string/string.hpp"

namespace arboris {
namespace {

inline bool IsValidPosition(std::string_view content, std::size_t pos) {
  return pos < content.length();
}

}  // anonymous namespace

std::size_t SkipWhitespace(std::string_view content, std::size_t begin) {
  while (IsValidPosition(content, begin) && std::isspace(content[begin])) {
    ++begin;
  }
  return begin;
}

std::string_view ExtractSubstring(std::string_view content, std::size_t start, std::size_t end) {
  if (start >= content.length() || start >= end) {
    return std::string_view{};
  }

  std::size_t actual_end = std::min(end, content.length());
  return content.substr(start, actual_end - start);
}

std::size_t FindNextChar(std::string_view content, std::size_t begin, char target_char) {
  for (std::size_t i = begin; i < content.length(); i++) {
    if (content[i] == target_char) {
      return i;
    }
  }
  return std::string::npos;
}

std::size_t FindNextAnyChar(std::string_view content, std::size_t begin, std::string_view target_chars) {
  for (std::size_t i = begin; i < content.length(); i++) {
    for (const char target : target_chars) {
      if (content[i] == target) {
        return i;
      }
    }
  }
  return std::string::npos;
}

std::size_t SkipUntilChar(std::string_view content, std::size_t begin, char target_char) {
  std::size_t found_pos = FindNextChar(content, begin, target_char);
  if (found_pos != std::string::npos) {
    return found_pos;
  }
  return std::string::npos;
}

}  // namespace arboris
