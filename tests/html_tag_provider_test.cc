/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/html_tag_provider.hpp"
#include "utils/html_tokens.hpp"
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
  void SetupTokenCollectors(HtmlTagProvider& provider, std::vector<HtmlToken>& open_tokens,
                            std::vector<HtmlTextToken>& text_tokens, std::vector<HtmlCloseToken>& close_tokens) {
    provider.set_feed_open_token_callback([&open_tokens](HtmlToken&& token) {
      open_tokens.push_back(std::move(token));
      return true;
    });

    provider.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
      text_tokens.push_back(std::move(token));
      return true;
    });

    provider.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token) {
      close_tokens.push_back(std::move(token));
      return true;
    });
  }

  // overloaded helper function using TokenCollectors struct
  void SetupTokenCollectors(HtmlTagProvider& provider, TokenCollectors& collectors) {
    SetupTokenCollectors(provider, collectors.open_tokens, collectors.text_tokens, collectors.close_tokens);
  }
};

/**
 * 1. Test basic Parse() behavior
 */
TEST_F(HtmlTagProviderTest, ParseEmptyString) {
  HtmlTagProvider provider(kEmptyString);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
  EXPECT_TRUE(tokens.open_tokens.empty());
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());
}

TEST_F(HtmlTagProviderTest, ParseTextOnly) {
  HtmlTagProvider provider(kTextOnly);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  // TODO(Team): Discuss with team about returning false for text-only content
  // Text-only content always causes Parse() to return false
  // because FindNextChar returns npos when '<' is not found
  EXPECT_FALSE(provider.Parse());

  // Even though parsing fails, text token should still be created
  EXPECT_TRUE(tokens.open_tokens.empty());
  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_TRUE(tokens.close_tokens.empty());

  EXPECT_EQ(tokens.text_tokens[0].text_content, "Hello World");
  EXPECT_EQ(tokens.text_tokens[0].begin_pos, 0);
}

TEST_F(HtmlTagProviderTest, ParseSimpleTag) {
  HtmlTagProvider provider(kSimpleTag);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
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
  HtmlTagProvider provider(kTagWithText);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
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
  HtmlTagProvider provider(kNestedTags);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
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
  HtmlTagProvider provider(kVoidTag);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());

  // Void tag validation
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kBr);
  EXPECT_TRUE(tokens.open_tokens[0].is_void_tag);
}

TEST_F(HtmlTagProviderTest, ParseMultipleVoidTags) {
  HtmlTagProvider provider(kMultipleVoidTags);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
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
  HtmlTagProvider provider(kTagWithAttributes);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());
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
  HtmlTagProvider provider(kComplexHtml);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());

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
  HtmlTagProvider provider(kUnclosedTag);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  // Unclosed tag with trailing text always causes Parse() to return false
  // because FindNextChar returns npos when no more '<' is found after the text
  EXPECT_FALSE(provider.Parse());

  // Even though parsing fails, tokens should still be created
  EXPECT_EQ(tokens.open_tokens.size(), 1);   // <div> tag parsed
  EXPECT_EQ(tokens.text_tokens.size(), 1);   // "content" text parsed
  EXPECT_TRUE(tokens.close_tokens.empty());  // no closing tag
}

TEST_F(HtmlTagProviderTest, ParseMalformedTagEmptyTag) {
  HtmlTagProvider provider(kEmptyTagName);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  // Empty tag name always causes Parse() to return false
  // because extractTagName returns empty string, causing parseOpenTag to return npos
  EXPECT_FALSE(provider.Parse());
}

TEST_F(HtmlTagProviderTest, ParseMalformedTagIncompleteTag) {
  HtmlTagProvider provider(kIncompleteTag);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  // Incomplete tag (missing '>') always causes Parse() to return false
  // because skipToTagEnd cannot find '>', causing parseOpenTag to return npos
  EXPECT_FALSE(provider.Parse());

  // No tokens should be created since tag parsing fails early
  EXPECT_TRUE(tokens.open_tokens.empty());
  EXPECT_TRUE(tokens.text_tokens.empty());
  EXPECT_TRUE(tokens.close_tokens.empty());
}

// Test callback return value
TEST_F(HtmlTagProviderTest, ParseCallbackReturnsFalseOpenToken) {
  HtmlTagProvider provider(kSimpleTag);

  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;

  // Set callback to return false for open token
  provider.set_feed_open_token_callback([&open_tokens](HtmlToken&& token) {
    open_tokens.push_back(std::move(token));
    return false;  // parsing stop
  });

  provider.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
    text_tokens.push_back(std::move(token));
    return true;
  });

  provider.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token) {
    close_tokens.push_back(std::move(token));
    return true;
  });

  EXPECT_FALSE(provider.Parse());    // false return due to parsing failure
  EXPECT_EQ(open_tokens.size(), 1);  // first token is processed
  EXPECT_TRUE(text_tokens.empty());  // subsequent tokens are not processed
  EXPECT_TRUE(close_tokens.empty());
}

TEST_F(HtmlTagProviderTest, ParseCallbackReturnsFalseTextToken) {
  HtmlTagProvider provider(kTagWithText);

  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;

  provider.set_feed_open_token_callback([&open_tokens](HtmlToken&& token) {
    open_tokens.push_back(std::move(token));
    return true;
  });

  // Set callback to return false for text token
  provider.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
    text_tokens.push_back(std::move(token));
    return false;  // parsing stop
  });

  provider.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token) {
    close_tokens.push_back(std::move(token));
    return true;
  });

  EXPECT_FALSE(provider.Parse());     // false return due to parsing failure
  EXPECT_EQ(open_tokens.size(), 1);   // Open tag is processed
  EXPECT_EQ(text_tokens.size(), 1);   // Text token is processed
  EXPECT_TRUE(close_tokens.empty());  // Close tag is not processed
}

TEST_F(HtmlTagProviderTest, ParseCallbackReturnsFalseCloseToken) {
  HtmlTagProvider provider(kSimpleTag);

  std::vector<HtmlToken> open_tokens;
  std::vector<HtmlTextToken> text_tokens;
  std::vector<HtmlCloseToken> close_tokens;

  provider.set_feed_open_token_callback([&open_tokens](HtmlToken&& token) {
    open_tokens.push_back(std::move(token));
    return true;
  });

  provider.set_feed_text_token_callback([&text_tokens](HtmlTextToken&& token) {
    text_tokens.push_back(std::move(token));
    return true;
  });

  // Set callback to return false for close token
  provider.set_feed_close_token_callback([&close_tokens](HtmlCloseToken&& token) {
    close_tokens.push_back(std::move(token));
    return false;  // parsing stop
  });

  EXPECT_FALSE(provider.Parse());     // false return due to parsing failure
  EXPECT_EQ(open_tokens.size(), 1);   // Open tag is processed
  EXPECT_TRUE(text_tokens.empty());   // Text is empty
  EXPECT_EQ(close_tokens.size(), 1);  // Close tag is processed
}

// Test parsing without callbacks
TEST_F(HtmlTagProviderTest, ParseNoCallbacks) {
  HtmlTagProvider provider(kComplexHtml);

  // Parse without callbacks
  EXPECT_TRUE(provider.Parse());  // Parse should succeed even without callbacks
}

// Test position information accuracy
TEST_F(HtmlTagProviderTest, ParsePositionAccuracy) {
  HtmlTagProvider provider(kPositionTest);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  EXPECT_TRUE(provider.Parse());

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
  HtmlTagProvider provider(kWhitespaceTest);
  TokenCollectors tokens;

  SetupTokenCollectors(provider, tokens);

  // Whitespace within tags should be handled correctly and parsing should succeed
  EXPECT_TRUE(provider.Parse());

  // All tokens should be created correctly despite whitespace
  EXPECT_EQ(tokens.open_tokens.size(), 1);
  EXPECT_EQ(tokens.open_tokens[0].tag, Tag::kDiv);

  EXPECT_EQ(tokens.text_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens.size(), 1);
  EXPECT_EQ(tokens.close_tokens[0].tag, Tag::kDiv);

  // Text content should preserve whitespace
  EXPECT_EQ(tokens.text_tokens[0].text_content, "  content  ");
}

}  // namespace arboris
