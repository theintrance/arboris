/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_STRING_STRING_HPP_
#define SRC_STRING_STRING_HPP_

#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>

namespace arboris {

/**
 * @brief Skip whitespace characters in a string_view starting from a given position
 * @param content The string content to process
 * @param pos Reference to current position, will be updated to skip whitespace
 */
inline void skip_whitespace(std::string_view content, std::uint32_t& pos) {
  while (pos < content.length() && std::isspace(content[pos]))
    pos++;
}

/**
 * @brief Trim whitespace from both ends of a string_view
 * @param content The string content to trim
 * @return Trimmed string_view
 */
inline std::string_view trim_whitespace(std::string_view content) {
  std::uint32_t start = 0;
  std::uint32_t end = content.length();

  while (start < end && std::isspace(content[start]))
    start++;
  while (start < end && std::isspace(content[end - 1]))
    end--;

  return content.substr(start, end - start);
}

/**
 * @brief Convert string to lowercase (in-place modification would require std::string)
 * @param content The string content to convert
 * @return Lowercase version of the string
 */
inline std::string to_lowercase(std::string_view content) {
  std::string result;
  result.reserve(content.length());

  for (const char c : content)
    result += std::tolower(c);

  return result;
}

/**
 * @brief Check if character at given position matches expected character
 * @param content The string content to check
 * @param pos Position to check
 * @param expected_char The character to compare against
 * @return True if position is valid and character matches
 */
inline bool is_char_at(std::string_view content, std::uint32_t pos, char expected_char) {
  return pos < content.length() && content[pos] == expected_char;
}

}  // namespace arboris

#endif  // SRC_STRING_STRING_HPP_
