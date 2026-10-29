/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_STRING_STRING_HPP_
#define SRC_STRING_STRING_HPP_

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace arboris {

// What HTML counts as whitespace inside a tag and between class names: tab, line feed, form
// feed, carriage return and space. Unlike std::isspace, no vertical tab.
inline constexpr std::string_view kHtmlWhitespace = " \t\n\f\r";

// A set of byte values, one bit each, so a scan tests a byte against the whole set at once.
class CharSet {
 public:
  constexpr CharSet() = default;

  constexpr explicit CharSet(std::string_view chars) {
    for (const char c : chars) {
      const auto byte = static_cast<unsigned char>(c);
      bits_[byte >> 6] |= std::uint64_t{1} << (byte & 63);
    }
  }

  [[nodiscard]] constexpr bool Contains(char c) const noexcept {
    const auto byte = static_cast<unsigned char>(c);
    return ((bits_[byte >> 6] >> (byte & 63)) & 1) != 0;
  }

 private:
  std::uint64_t bits_[4] = {};
};

inline constexpr CharSet kHtmlWhitespaceSet{kHtmlWhitespace};

// Position of the first byte at or after begin that is in the set, or npos.
std::size_t FindNextOf(std::string_view content, std::size_t begin, const CharSet& set);

// Position of the first byte at or after begin that is not in the set, or content.length().
std::size_t SkipAll(std::string_view content, std::size_t begin, const CharSet& set);

// Skips std::isspace characters.
std::size_t SkipWhitespace(std::string_view content, std::size_t begin);

std::size_t SkipHtmlWhitespace(std::string_view content, std::size_t begin);

// content[start, end), clipped to the content; empty when start is past end or the content.
std::string_view ExtractSubstring(std::string_view content, std::size_t start, std::size_t end);

// Position of the first target_char at or after begin, or npos.
std::size_t FindNextChar(std::string_view content, std::size_t begin, char target_char);

// Position of the first byte at or after begin that is one of target_chars, or npos.
std::size_t FindNextAnyChar(std::string_view content, std::size_t begin, std::string_view target_chars);

std::size_t SkipUntilChar(std::string_view content, std::size_t begin, char target_char);

}  // namespace arboris

#endif  // SRC_STRING_STRING_HPP_
