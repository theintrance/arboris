/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "string/string.hpp"

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

}  // namespace arboris
