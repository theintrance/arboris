/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/html_token_parser.hpp"
#include "utils/html_tokens.hpp"
#include "utils/string_pool.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

// test data
constexpr std::string_view kEmptyString = "";
constexpr std::string_view kSimpleTag = "<div></div>";
constexpr std::string_view kTagWithText = "<p>Hello World</p>";
constexpr std::string_view kNestedTags = "<div><span>nested</span></div>";
constexpr std::string_view kVoidTag = "<br>";
constexpr std::string_view kMultipleVoidTags = "<br><img><input>";
constexpr std::string_view kTagWithAttributes = "<div class='test' id='main'>content</div>";
constexpr std::string_view kComplexHtml = "<html><head><title>Test</title></head><body><div>Hello</div></body></html>";
constexpr std::string_view kNestedTagsWithText = "<p>Hello<b>content</b>World</p>";

// test data for edge cases and error conditions
constexpr std::string_view kTextOnly = "Hello World";
constexpr std::string_view kUnclosedTag = "<div>content";
constexpr std::string_view kEmptyTagName = "<>content</>";
constexpr std::string_view kIncompleteTag = "<div";
constexpr std::string_view kPositionTest = "<p>test</p>";
constexpr std::string_view kWhitespaceTest = "<  div  >  content  </  div  >";

}  // anonymous namespace

// Helper struct to reduce boilerplate code
struct TokenCollectors {
  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;
};

class HtmlTagProviderTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  // helper function: set callback for collecting tokens
  void SetupTokenCollectors(HtmlTokenParser& parser, std::vector<HtmlToken>& open_tokens,
                            std::vector<HtmlTextToken>& text_tokens, std::vector<HtmlCloseToken>& close_tokens) {
    parser.set_feed_open_token_callback([&open_tokens](HtmlToken&& token, const char*) {
      open_tokens.push_back(std::move(token));
      return true;
    });

    parser.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
      text_tokens.push_back(std::move(token));
      return true;
    });

    parser.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token, const char*) {
      close_tokens.push_back(std::move(token));
      return true;
    });
  }

  // overloaded helper function using TokenCollectors struct
  void SetupTokenCollectors(HtmlTokenParser& parser, TokenCollectors& collectors) {
    SetupTokenCollectors(parser, collectors.open_tokens, collectors.text_tokens, collectors.close_tokens);
  }
};

/**
 * 1. Test basic Parse() behavior
 */
TEST_F(HtmlTagProviderTest, ParseEmptyString) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kEmptyString, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_TRUE(tokens.open_tokens.empty());
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());
}

TEST_F(HtmlTagProviderTest, ParseTextOnly) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kTextOnly, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  // TODO(Team): Discuss with team about returning false for text-only content
  // Text-only content always causes Parse() to return false
  // because FindNextChar returns npos when '<' is not found
  EXPECT_FALSE(parser.Parse());

  // Even though parsing fails, text token should still be created
  EXPECT_TRUE(tokens.open_tokens.empty());
  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_TRUE(tokens.close_tokens.empty());

  EXPECT_EQ(tokens.text_tokens[0].text_content, "Hello World");
  EXPECT_EQ(tokens.text_tokens[0].begin_pos, 0);
}

TEST_F(HtmlTagProviderTest, ParseSimpleTag) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kSimpleTag, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_EQ(tokens.close_tokens.size(), 1);

  // Open tag validation
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kDiv);
  EXPECT_FALSE(tokens.open_tokens[0].is_void_tag);
  EXPECT_EQ(tokens.open_tokens[0].begin_pos, 0);
  EXPECT_EQ(tokens.open_tokens[0].end_pos, 5);

  // Close tag validation
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kDiv);
  EXPECT_EQ(tokens.close_tokens[0].begin_pos, 5);
  EXPECT_EQ(tokens.close_tokens[0].end_pos, 11);
}

TEST_F(HtmlTagProviderTest, ParseTagWithText) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kTagWithText, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens.size(), 1);

  // Open tag validation
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kP);
  EXPECT_FALSE(tokens.open_tokens[0].is_void_tag);

  // Text validation
  EXPECT_EQ(tokens.text_tokens[0].text_content, "Hello World");

  // Close tag validation
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kP);
}

TEST_F(HtmlTagProviderTest, ParseNestedTags) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kNestedTags, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 2);
  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens.size(), 2);

  // first open tag (div)
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kDiv);

  // second open tag (span)
  EXPECT_EQ(tokens.open_tokens[1].tag, Tag::kSpan);

  // Text
  EXPECT_EQ(tokens.text_tokens[0].text_content, "nested");

  // Close tags (span, div order)
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kSpan);
  EXPECT_EQ(tokens.close_tokens[1].tag, Tag::kDiv);
}

TEST_F(HtmlTagProviderTest, ParseVoidTag) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kVoidTag, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());

  // Void tag validation
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kBr);
  EXPECT_TRUE(tokens.open_tokens[0].is_void_tag);
}

TEST_F(HtmlTagProviderTest, ParseMultipleVoidTags) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kMultipleVoidTags, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 3);
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());

  // Each void tag validation
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kBr);
  EXPECT_TRUE(tokens.open_tokens[0].is_void_tag);

  EXPECT_EQ(tokens.open_tokens[1].tag, Tag::kImg);
  EXPECT_TRUE(tokens.open_tokens[1].is_void_tag);

  EXPECT_EQ(tokens.open_tokens[2].tag, Tag::kInput);
  EXPECT_TRUE(tokens.open_tokens[2].is_void_tag);
}

TEST_F(HtmlTagProviderTest, ParseTagWithAttributes) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kTagWithAttributes, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens.size(), 1);

  // Even if attributes are present, the tag name should be parsed correctly
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kDiv);
  EXPECT_EQ(tokens.text_tokens[0].text_content, "content");
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kDiv);

  // TODO(Team): Add comprehensive attribute parsing tests
  // - Test attribute extraction and validation
  // - Test multiple attributes with different formats
  // - Test attribute values with quotes (single/double)
  // - Test malformed attributes handling
}

TEST_F(HtmlTagProviderTest, ParseComplexHtml) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kComplexHtml, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());

  // Expected tags: html, head, title, body, div
  EXPECT_EQ(tokens.open_tokens.size(), 5);
  EXPECT_EQ(tokens.close_tokens.size(), 5);

  // Expected text: "Test", "Hello"
  EXPECT_EQ(tokens.text_tokens.size(), 2);

  // Tag order validation
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kHtml);
  EXPECT_EQ(tokens.open_tokens[1].tag, Tag::kHead);
  EXPECT_EQ(tokens.open_tokens[2].tag, Tag::kTitle);
  EXPECT_EQ(tokens.open_tokens[3].tag, Tag::kBody);
  EXPECT_EQ(tokens.open_tokens[4].tag, Tag::kDiv);

  // Text content validation
  EXPECT_EQ(tokens.text_tokens[0].text_content, "Test");
  EXPECT_EQ(tokens.text_tokens[1].text_content, "Hello");

  // Close tag order validation (should be reverse of open tags)
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kTitle);
  EXPECT_EQ(tokens.close_tokens[1].tag, Tag::kHead);
  EXPECT_EQ(tokens.close_tokens[2].tag, Tag::kDiv);
  EXPECT_EQ(tokens.close_tokens[3].tag, Tag::kBody);
  EXPECT_EQ(tokens.close_tokens[4].tag, Tag::kHtml);

  // Position validation for key elements
  // <html> tag positions
  EXPECT_EQ(tokens.open_tokens[0].begin_pos, 0);
  EXPECT_EQ(tokens.open_tokens[0].end_pos, 6);

  // <title> tag positions
  EXPECT_EQ(tokens.open_tokens[2].begin_pos, 12);
  EXPECT_EQ(tokens.open_tokens[2].end_pos, 19);

  // First text token "Test" positions
  EXPECT_EQ(tokens.text_tokens[0].begin_pos, 19);
  EXPECT_EQ(tokens.text_tokens[0].end_pos, 23);

  // </title> close tag positions
  EXPECT_EQ(tokens.close_tokens[0].begin_pos, 23);
  EXPECT_EQ(tokens.close_tokens[0].end_pos, 31);
}

// Test error cases
TEST_F(HtmlTagProviderTest, ParseMalformedTagUnclosedTag) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kUnclosedTag, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  // Unclosed tag with trailing text always causes Parse() to return false
  // because FindNextChar returns npos when no more '<' is found after the text
  EXPECT_FALSE(parser.Parse());

  // Even though parsing fails, tokens should still be created
  EXPECT_EQ(tokens.open_tokens.size(), 1);   // <div> tag parsed
  EXPECT_EQ(tokens.text_tokens.size(), 1);   // "content" text parsed
  EXPECT_TRUE(tokens.close_tokens.empty());  // no closing tag
}

TEST_F(HtmlTagProviderTest, ParseMalformedTagEmptyTag) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kEmptyTagName, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  // Empty tag name always causes Parse() to return false
  // because extractTagName returns empty string, causing parseOpenTag to return npos
  EXPECT_FALSE(parser.Parse());
}

TEST_F(HtmlTagProviderTest, ParseMalformedTagIncompleteTag) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kIncompleteTag, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  // Incomplete tag (missing '>') always causes Parse() to return false
  // because skipToTagEnd cannot find '>', causing parseOpenTag to return npos
  EXPECT_FALSE(parser.Parse());

  // No tokens should be created since tag parsing fails early
  EXPECT_TRUE(tokens.open_tokens.empty());
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());
}

// Test callback return value
TEST_F(HtmlTagProviderTest, ParseCallbackReturnsFalseOpenToken) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kSimpleTag, string_pool);

  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;

  // Set callback to return false for open token
  parser.set_feed_open_token_callback([&open_tokens](HtmlToken&& token, const char*) {
    open_tokens.push_back(std::move(token));
    return false;  // parsing stop
  });

  parser.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
    text_tokens.push_back(std::move(token));
    return true;
  });

  parser.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token, const char*) {
    close_tokens.push_back(std::move(token));
    return true;
  });

  EXPECT_FALSE(parser.Parse());      // false return due to parsing failure
  EXPECT_EQ(open_tokens.size(), 1);  // first token is processed
  EXPECT_TRUE(text_tokens.empty());  // subsequent tokens are not processed
  EXPECT_TRUE(close_tokens.empty());
}

TEST_F(HtmlTagProviderTest, ParseCallbackReturnsFalseTextToken) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kTagWithText, string_pool);

  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;

  parser.set_feed_open_token_callback([&open_tokens](HtmlToken&& token, const char*) {
    open_tokens.push_back(std::move(token));
    return true;
  });

  // Set callback to return false for text token
  parser.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
    text_tokens.push_back(std::move(token));
    return false;  // parsing stop
  });

  parser.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token, const char*) {
    close_tokens.push_back(std::move(token));
    return true;
  });

  EXPECT_FALSE(parser.Parse());       // false return due to parsing failure
  EXPECT_EQ(open_tokens.size(), 1);   // Open tag is processed
  EXPECT_EQ(text_tokens.size(), 1);   // Text token is processed
  EXPECT_TRUE(close_tokens.empty());  // Close tag is not processed
}

TEST_F(HtmlTagProviderTest, ParseCallbackReturnsFalseCloseToken) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kSimpleTag, string_pool);

  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;

  parser.set_feed_open_token_callback([&open_tokens](HtmlToken&& token, const char*) {
    open_tokens.push_back(std::move(token));
    return true;
  });

  parser.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
    text_tokens.push_back(std::move(token));
    return true;
  });

  // Set callback to return false for close token
  parser.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token, const char*) {
    close_tokens.push_back(std::move(token));
    return false;  // parsing stop
  });

  EXPECT_FALSE(parser.Parse());       // false return due to parsing failure
  EXPECT_EQ(open_tokens.size(), 1);   // Open tag is processed
  EXPECT_TRUE(text_tokens.empty());   // Text is empty
  EXPECT_EQ(close_tokens.size(), 1);  // Close tag is processed
}

// Test parsing without callbacks
TEST_F(HtmlTagProviderTest, ParseNoCallbacks) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kComplexHtml, string_pool);

  // Parse without callbacks
  EXPECT_TRUE(parser.Parse());  // Parse should succeed even without callbacks
}

// Test position information accuracy
TEST_F(HtmlTagProviderTest, ParsePositionAccuracy) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kPositionTest, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());

  // Position accuracy validation
  EXPECT_EQ(tokens.open_tokens[0].begin_pos, 0);  // <p>
  EXPECT_EQ(tokens.open_tokens[0].end_pos, 3);

  EXPECT_EQ(tokens.text_tokens[0].begin_pos, 3);  // test
  EXPECT_EQ(tokens.text_tokens[0].end_pos, 7);

  EXPECT_EQ(tokens.close_tokens[0].begin_pos, 7);  // </p>
  EXPECT_EQ(tokens.close_tokens[0].end_pos, 11);
}

// Test whitespace handling within tags and text
TEST_F(HtmlTagProviderTest, ParseWhitespaceHandling) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kWhitespaceTest, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  // Whitespace within tags should be handled correctly and parsing should succeed
  EXPECT_TRUE(parser.Parse());

  // All tokens should be created correctly despite whitespace
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kDiv);

  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kDiv);

  // Text content should preserve whitespace
  EXPECT_EQ(tokens.text_tokens[0].text_content, "  content  ");
}

// Test nested tags with mixed text content
TEST_F(HtmlTagProviderTest, ParseNestedTagsWithMixedText) {
  auto string_pool = std::make_shared<StringPool>(1024);
  HtmlTokenParser parser(kNestedTagsWithText, string_pool);
  TokenCollectors tokens;

  SetupTokenCollectors(parser, tokens);

  EXPECT_TRUE(parser.Parse());

  // Expected structure: <p>Hello<b>content</b>World</p>
  // Open tags: p, b
  EXPECT_EQ(tokens.open_tokens.size(), 2);
  // Text tokens: "Hello", "content", "World"
  EXPECT_EQ(tokens.text_tokens.size(), 3);
  // Close tags: b, p
  EXPECT_EQ(tokens.close_tokens.size(), 2);

  // Validate open tags order
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kP);
  EXPECT_FALSE(tokens.open_tokens[0].is_void_tag);
  EXPECT_EQ(tokens.open_tokens[1].tag, Tag::kB);
  EXPECT_FALSE(tokens.open_tokens[1].is_void_tag);

  // Validate text tokens content and order
  EXPECT_EQ(tokens.text_tokens[0].text_content, "Hello");
  EXPECT_EQ(tokens.text_tokens[1].text_content, "content");
  EXPECT_EQ(tokens.text_tokens[2].text_content, "World");

  // Validate close tags order (should be reverse of open tags)
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kB);
  EXPECT_EQ(tokens.close_tokens[1].tag, Tag::kP);

  // Validate position information for key elements
  // <p> tag positions
  EXPECT_EQ(tokens.open_tokens[0].begin_pos, 0);
  EXPECT_EQ(tokens.open_tokens[0].end_pos, 3);

  // First text "Hello" positions
  EXPECT_EQ(tokens.text_tokens[0].begin_pos, 3);
  EXPECT_EQ(tokens.text_tokens[0].end_pos, 8);

  // <b> tag positions
  EXPECT_EQ(tokens.open_tokens[1].begin_pos, 8);
  EXPECT_EQ(tokens.open_tokens[1].end_pos, 11);

  // Second text "content" positions
  EXPECT_EQ(tokens.text_tokens[1].begin_pos, 11);
  EXPECT_EQ(tokens.text_tokens[1].end_pos, 18);

  // </b> close tag positions
  EXPECT_EQ(tokens.close_tokens[0].begin_pos, 18);
  EXPECT_EQ(tokens.close_tokens[0].end_pos, 22);

  // Third text "World" positions
  EXPECT_EQ(tokens.text_tokens[2].begin_pos, 22);
  EXPECT_EQ(tokens.text_tokens[2].end_pos, 27);

  // </p> close tag positions
  EXPECT_EQ(tokens.close_tokens[1].begin_pos, 27);
  EXPECT_EQ(tokens.close_tokens[1].end_pos, 31);
}

}  // namespace arboris
