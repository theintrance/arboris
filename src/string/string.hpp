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

/**
 * @brief Skip whitespace characters using size_t position (for compatibility)
 * @param content The string content to process
 * @param pos Reference to current position, will be updated to skip whitespace
 */
void SkipWhitespace(std::string_view content, std::size_t& pos);

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
 * @param pos Starting position for search
 * @param target_char Character to find
 * @return Position of found character, or content.length() if not found
 */
std::size_t FindNextChar(std::string_view content, std::size_t pos, char target_char);

/**
 * @brief Find next occurrence of any character in a set, starting from given position
 * @param content The string content to search in
 * @param pos Starting position for search
 * @param target_chars Set of characters to find (null-terminated string)
 * @return Position of found character, or content.length() if not found
 */
std::size_t FindNextAnyChar(std::string_view content, std::size_t pos, const char* target_chars);

/**
 * @brief Skip characters until a specific character is found
 * @param content The string content to process
 * @param pos Reference to current position, will be updated
 * @param target_char Character to stop at
 * @return True if target character was found, false if end of string reached
 */
bool SkipUntilChar(std::string_view content, std::size_t& pos, char target_char);

/**
 * @brief Check if position is at the end of content
 * @param content The string content to check
 * @param pos Position to check
 * @return True if position is at or beyond end of content
 */
bool IsAtEnd(std::string_view content, std::size_t pos);

}  // namespace arboris

#endif  // SRC_STRING_STRING_HPP_
