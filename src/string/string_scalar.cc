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
namespace {

inline bool IsValidPosition(std::string_view content, std::size_t pos) {
  return pos < content.length();
}

template <typename PosType>
inline void SkipWhitespaceImpl(std::string_view content, PosType& pos) {
  while (IsValidPosition(content, pos) && std::isspace(content[pos]))
    ++pos;
}

template <typename Predicate>
std::size_t FindNextCharImpl(std::string_view content, std::size_t pos, Predicate pred) {
  while (IsValidPosition(content, pos)) {
    if (pred(content[pos]))
      return pos;
    ++pos;
  }
  return std::string::npos;
}

}  // anonymous namespace

void SkipWhitespace(std::string_view content, std::uint32_t& pos) {
  SkipWhitespaceImpl(content, pos);
}

void SkipWhitespace(std::string_view content, std::size_t& pos) {
  SkipWhitespaceImpl(content, pos);
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
  return IsValidPosition(content, pos) && content[pos] == expected_char;
}

std::string_view ExtractSubstring(std::string_view content, std::size_t start, std::size_t end) {
  if (start >= content.length() || start >= end)
    return std::string_view();

  std::size_t actual_end = std::min(end, content.length());
  return content.substr(start, actual_end - start);
}

std::size_t FindNextChar(std::string_view content, std::size_t pos, char target_char) {
  return FindNextCharImpl(content, pos, [target_char](char c) { return c == target_char; });
}

std::size_t FindNextAnyChar(std::string_view content, std::size_t pos, const char* target_chars) {
  return FindNextCharImpl(content, pos, [target_chars](char current_char) {
    for (const char* target = target_chars; *target != '\0'; ++target) {
      if (current_char == *target)
        return true;
    }
    return false;
  });
}

bool SkipUntilChar(std::string_view content, std::size_t& pos, char target_char) {
  std::size_t found_pos = FindNextChar(content, pos, target_char);
  if (found_pos != std::string::npos) {
    pos = found_pos;
    return true;
  }
  pos = content.length();
  return false;
}

bool IsAtEnd(std::string_view content, std::size_t pos) {
  return !IsValidPosition(content, pos);
}

}  // namespace arboris
