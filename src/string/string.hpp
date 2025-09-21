/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_STRING_STRING_HPP_
#define SRC_STRING_STRING_HPP_

#include <cctype>
#include <string_view>

namespace arboris {

/**
 * @brief Skip whitespace characters in a string_view starting from a given position
 * @param content The string content to process
 * @param begin Starting position for skipping whitespace
 * @return Position after skipping whitespace
 */
std::size_t SkipWhitespace(std::string_view content, std::size_t begin);

/**
 * @brief Extract substring from content using start and end positions
 * @param content The string content to extract from
 * @param start Starting position (inclusive)
 * @param end Ending position (exclusive)
 * @return Extracted string_view
 */
std::string_view ExtractSubstring(std::string_view content, std::size_t start, std::size_t end);

/**
 * @brief Find next occurrence of character starting from given position
 * @param content The string content to search in
 * @param begin Starting position for search
 * @param target_char Character to find
 * @return Position of found character, or std::string::npos if not found
 */
std::size_t FindNextChar(std::string_view content, std::size_t begin, char target_char);

/**
 * @brief Find next occurrence of any character in a set, starting from given position
 * @param content The string content to search in
 * @param begin Starting position for search
 * @param target_chars Set of characters to find (null-terminated string)
 * @return Position of found character, or std::string::npos if not found
 */
std::size_t FindNextAnyChar(std::string_view content, std::size_t begin, const char* target_chars);

/**
 * @brief Skip characters until a specific character is found
 * @param content The string content to process
 * @param begin Starting position for search
 * @param target_char Character to stop at
 * @return Position of found character, or std::string::npos if not found
 */
std::size_t SkipUntilChar(std::string_view content, std::size_t begin, char target_char);

}  // namespace arboris

#endif  // SRC_STRING_STRING_HPP_
