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
void SkipWhitespace(std::string_view content, std::uint32_t& pos);

/**
 * @brief Trim whitespace from both ends of a string_view
 * @param content The string content to trim
 * @return Trimmed string_view
 */
std::string_view TrimWhitespace(std::string_view content);

/**
 * @brief Convert string to lowercase (in-place modification would require std::string)
 * @param content The string content to convert
 * @return Lowercase version of the string
 */
std::string ToLowercase(std::string_view content);

/**
 * @brief Check if character at given position matches expected character
 * @param content The string content to check
 * @param pos Position to check
 * @param expected_char The character to compare against
 * @return True if position is valid and character matches
 */
bool IsCharAt(std::string_view content, std::uint32_t pos, char expected_char);

}  // namespace arboris

#endif  // SRC_STRING_STRING_HPP_
