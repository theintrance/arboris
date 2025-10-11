/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <string_view>

#include "string/string.hpp"

namespace arboris {
namespace {

// Test data constants
constexpr std::string_view kEmptyString = "";
constexpr std::string_view kWhitespaceString = "   \t\n\r  ";
constexpr std::string_view kMixedString = "hello world\ttest\n";
constexpr std::string_view kNoWhitespaceString = "helloworld";
constexpr std::string_view kSpecialChars = "!@#$%^&*()";

// Additional test data for inline strings
constexpr std::string_view kHelloString = "hello";
constexpr std::string_view kWorldString = "world";
constexpr std::string_view kHelloWorldString = "hello world";
constexpr std::string_view kSpacedHelloString = "   hello";
constexpr std::string_view kTabNewlineHelloString = "\t\n hello";
constexpr std::string_view kMixedWhitespaceHelloString = " \r\n\t hello";

}  // anonymous namespace

class StringUtilsTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

// SkipWhitespace tests
TEST_F(StringUtilsTest, SkipWhitespaceEmptyString) {
  EXPECT_EQ(SkipWhitespace(kEmptyString, 0), 0);
}

TEST_F(StringUtilsTest, SkipWhitespaceNoWhitespace) {
  EXPECT_EQ(SkipWhitespace(kNoWhitespaceString, 0), 0);
  EXPECT_EQ(SkipWhitespace(kHelloString, 2), 2);  // Start from middle
}

TEST_F(StringUtilsTest, SkipWhitespaceWithWhitespace) {
  EXPECT_EQ(SkipWhitespace(kSpacedHelloString, 0), 3);
  EXPECT_EQ(SkipWhitespace(kTabNewlineHelloString, 0), 3);
  EXPECT_EQ(SkipWhitespace(kMixedWhitespaceHelloString, 0), 5);
}

TEST_F(StringUtilsTest, SkipWhitespaceAllWhitespace) {
  EXPECT_EQ(SkipWhitespace(kWhitespaceString, 0), kWhitespaceString.length());
}

TEST_F(StringUtilsTest, SkipWhitespaceBoundaryConditions) {
  // Start from end of string
  EXPECT_EQ(SkipWhitespace(kHelloWorldString, kHelloWorldString.length()), kHelloWorldString.length());

  // Position beyond string length
  EXPECT_EQ(SkipWhitespace(kHelloWorldString, kHelloWorldString.length() + 1), kHelloWorldString.length() + 1);

  // Maximum size_t value (overflow test)
  EXPECT_EQ(SkipWhitespace(kHelloWorldString, std::numeric_limits<std::size_t>::max()),
            std::numeric_limits<std::size_t>::max());
}

// ExtractSubstring tests
TEST_F(StringUtilsTest, ExtractSubstringNormalCases) {
  EXPECT_EQ(ExtractSubstring(kHelloWorldString, 0, 5), kHelloString);
  EXPECT_EQ(ExtractSubstring(kHelloWorldString, 6, 11), kWorldString);
  EXPECT_EQ(ExtractSubstring(kHelloWorldString, 0, kHelloWorldString.length()), kHelloWorldString);
}

TEST_F(StringUtilsTest, ExtractSubstringEmptyResults) {
  // start == end
  EXPECT_EQ(ExtractSubstring(kHelloString, 2, 2), kEmptyString);

  // start > end
  EXPECT_EQ(ExtractSubstring(kHelloString, 3, 2), kEmptyString);

  // start >= length
  EXPECT_EQ(ExtractSubstring(kHelloString, 5, 10), kEmptyString);
  EXPECT_EQ(ExtractSubstring(kHelloString, 10, 15), kEmptyString);
}

TEST_F(StringUtilsTest, ExtractSubstringEmptyString) {
  EXPECT_EQ(ExtractSubstring(kEmptyString, 0, 0), "");
  EXPECT_EQ(ExtractSubstring(kEmptyString, 0, 5), "");
  EXPECT_EQ(ExtractSubstring(kEmptyString, 5, 10), "");
}

TEST_F(StringUtilsTest, ExtractSubstringEndBeyondLength) {
  std::string_view test_str = "hello";

  // When end > length, return up to the entire string
  EXPECT_EQ(ExtractSubstring(test_str, 0, 10), "hello");
  EXPECT_EQ(ExtractSubstring(test_str, 2, 10), "llo");
}

TEST_F(StringUtilsTest, ExtractSubstringBoundaryConditions) {
  std::string_view test_str = "test";

  // Maximum size_t values
  EXPECT_EQ(
      ExtractSubstring(test_str, std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max()), "");

  // Large end value
  EXPECT_EQ(ExtractSubstring(test_str, 0, std::numeric_limits<std::size_t>::max()), "test");
}

// FindNextChar tests
TEST_F(StringUtilsTest, FindNextCharFound) {
  std::string_view test_str = "hello world";

  EXPECT_EQ(FindNextChar(test_str, 0, 'h'), 0);
  EXPECT_EQ(FindNextChar(test_str, 0, 'e'), 1);
  EXPECT_EQ(FindNextChar(test_str, 0, 'l'), 2);  // First 'l'
  EXPECT_EQ(FindNextChar(test_str, 3, 'l'), 3);  // Second 'l'
  EXPECT_EQ(FindNextChar(test_str, 0, ' '), 5);
}

TEST_F(StringUtilsTest, FindNextCharNotFound) {
  std::string_view test_str = "hello world";

  EXPECT_EQ(FindNextChar(test_str, 0, 'x'), std::string::npos);
  EXPECT_EQ(FindNextChar(test_str, 0, 'z'), std::string::npos);

  // Character not found after start position
  EXPECT_EQ(FindNextChar(test_str, 6, 'h'), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextCharEmptyString) {
  EXPECT_EQ(FindNextChar(kEmptyString, 0, 'a'), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextCharBoundaryConditions) {
  std::string_view test_str = "hello";

  // Start from end of string
  EXPECT_EQ(FindNextChar(test_str, test_str.length(), 'h'), std::string::npos);

  // Position beyond string length
  EXPECT_EQ(FindNextChar(test_str, test_str.length() + 1, 'h'), std::string::npos);

  // Maximum size_t value
  EXPECT_EQ(FindNextChar(test_str, std::numeric_limits<std::size_t>::max(), 'h'), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextCharSpecialCharacters) {
  std::string_view test_str = "hello\tworld\n";

  EXPECT_EQ(FindNextChar(test_str, 0, '\t'), 5);
  EXPECT_EQ(FindNextChar(test_str, 0, '\n'), 11);
  EXPECT_EQ(FindNextChar(test_str, 0, '\0'), std::string::npos);
}

// FindNextAnyChar tests
TEST_F(StringUtilsTest, FindNextAnyCharFound) {
  std::string_view test_str = "hello world";

  EXPECT_EQ(FindNextAnyChar(test_str, 0, "aeiou"), 1);  // 'e'
  EXPECT_EQ(FindNextAnyChar(test_str, 2, "aeiou"), 4);  // 'o'
  EXPECT_EQ(FindNextAnyChar(test_str, 0, "hw"), 0);     // 'h'
  EXPECT_EQ(FindNextAnyChar(test_str, 1, "hw"), 6);     // 'w'
}

TEST_F(StringUtilsTest, FindNextAnyCharNotFound) {
  std::string_view test_str = "hello world";

  EXPECT_EQ(FindNextAnyChar(test_str, 0, "xyz"), std::string::npos);
  EXPECT_EQ(FindNextAnyChar(test_str, 0, "123"), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextAnyCharEmptyTargets) {
  std::string_view test_str = "hello";

  EXPECT_EQ(FindNextAnyChar(test_str, 0, ""), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextAnyCharEmptyString) {
  EXPECT_EQ(FindNextAnyChar(kEmptyString, 0, "abc"), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextAnyCharBoundaryConditions) {
  std::string_view test_str = "hello";

  // Start from end of string
  EXPECT_EQ(FindNextAnyChar(test_str, test_str.length(), "helo"), std::string::npos);

  // Position beyond string length
  EXPECT_EQ(FindNextAnyChar(test_str, test_str.length() + 1, "helo"), std::string::npos);

  // Maximum size_t value
  EXPECT_EQ(FindNextAnyChar(test_str, std::numeric_limits<std::size_t>::max(), "helo"), std::string::npos);
}

TEST_F(StringUtilsTest, FindNextAnyCharSingleChar) {
  std::string_view test_str = "hello";

  EXPECT_EQ(FindNextAnyChar(test_str, 0, "h"), 0);
  EXPECT_EQ(FindNextAnyChar(test_str, 0, "e"), 1);
}

// SkipUntilChar tests
TEST_F(StringUtilsTest, SkipUntilCharFound) {
  std::string_view test_str = "hello world";

  EXPECT_EQ(SkipUntilChar(test_str, 0, ' '), 5);
  EXPECT_EQ(SkipUntilChar(test_str, 0, 'w'), 6);
  EXPECT_EQ(SkipUntilChar(test_str, 0, 'h'), 0);  // First character
}

TEST_F(StringUtilsTest, SkipUntilCharNotFound) {
  std::string_view test_str = "hello world";

  EXPECT_EQ(SkipUntilChar(test_str, 0, 'x'), std::string::npos);
  EXPECT_EQ(SkipUntilChar(test_str, 0, 'z'), std::string::npos);
}

TEST_F(StringUtilsTest, SkipUntilCharEmptyString) {
  EXPECT_EQ(SkipUntilChar(kEmptyString, 0, 'a'), std::string::npos);
}

TEST_F(StringUtilsTest, SkipUntilCharBoundaryConditions) {
  std::string_view test_str = "hello";

  // Start from end of string
  EXPECT_EQ(SkipUntilChar(test_str, test_str.length(), 'h'), std::string::npos);

  // Position beyond string length
  EXPECT_EQ(SkipUntilChar(test_str, test_str.length() + 1, 'h'), std::string::npos);

  // Maximum size_t value
  EXPECT_EQ(SkipUntilChar(test_str, std::numeric_limits<std::size_t>::max(), 'h'), std::string::npos);
}

TEST_F(StringUtilsTest, SkipUntilCharConsistencyWithFindNextChar) {
  std::string_view test_str = "hello world test";

  // SkipUntilChar should return the same result as FindNextChar
  EXPECT_EQ(SkipUntilChar(test_str, 0, ' '), FindNextChar(test_str, 0, ' '));
  EXPECT_EQ(SkipUntilChar(test_str, 5, 't'), FindNextChar(test_str, 5, 't'));
  EXPECT_EQ(SkipUntilChar(test_str, 0, 'x'), FindNextChar(test_str, 0, 'x'));
}

// Integration safety tests
TEST_F(StringUtilsTest, IntegrationSafetyChainedOperations) {
  std::string_view test_str = "  hello world  ";

  // Verify that chained operations work safely
  std::size_t start = SkipWhitespace(test_str, 0);
  EXPECT_EQ(start, 2);

  std::size_t space_pos = FindNextChar(test_str, start, ' ');
  EXPECT_EQ(space_pos, 7);

  std::string_view word = ExtractSubstring(test_str, start, space_pos);
  EXPECT_EQ(word, "hello");

  std::size_t next_start = SkipWhitespace(test_str, space_pos);
  EXPECT_EQ(next_start, 8);
}

TEST_F(StringUtilsTest, IntegrationSafetyEdgeCaseChaining) {
  std::string_view empty_str = "";

  // Chained operations on empty string
  std::size_t pos1 = SkipWhitespace(empty_str, 0);
  std::size_t pos2 = FindNextChar(empty_str, pos1, 'a');
  std::string_view result = ExtractSubstring(empty_str, pos1, pos2);

  EXPECT_EQ(pos1, 0);
  EXPECT_EQ(pos2, std::string::npos);
  EXPECT_EQ(result, "");
}

// Memory safety test (prevent dangling pointers)
TEST_F(StringUtilsTest, MemorySafetyStringViewLifetime) {
  std::string_view result;

  {
    std::string temp_str = "temporary string";
    std::string_view temp_view = temp_str;
    result = ExtractSubstring(temp_view, 0, 9);
    // Use result before temp_str goes out of scope
    EXPECT_EQ(result, "temporary");
  }

  // Using result here could create a dangling pointer
  // This is something users need to be careful about, so we don't test it
}

}  // namespace arboris
