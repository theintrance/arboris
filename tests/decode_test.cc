/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "string/decode.hpp"

namespace arboris {
namespace {

std::string Decoded(std::string_view in, TextMode mode = TextMode::kText) {
  std::string out;
  AppendDecoded(in, mode, &out);
  return out;
}

TEST(DecodeTest, NamedReferences) {
  EXPECT_EQ(Decoded("x &amp; y"), "x & y");
  EXPECT_EQ(Decoded("&lt;p&gt;"), "<p>");
  EXPECT_EQ(Decoded("&nbsp;"), "\xc2\xa0");
  EXPECT_EQ(Decoded("&nGt;"), "\xe2\x89\xab\xe2\x83\x92");
  EXPECT_EQ(Decoded("&nosuch;"), "&nosuch;");
  EXPECT_EQ(Decoded("&Abreve"), "&Abreve");
}

TEST(DecodeTest, LegacyReferencesNeedNoSemicolon) {
  EXPECT_EQ(Decoded("&copy 2026"), "\xc2\xa9 2026");
  EXPECT_EQ(Decoded("I'm &notit; I tell you"), "I'm \xc2\xac" "it; I tell you");
  EXPECT_EQ(Decoded("I'm &notin; I tell you"), "I'm \xe2\x88\x89 I tell you");
}

TEST(DecodeTest, AttributeValuesKeepAmbiguousLegacyReferences) {
  EXPECT_EQ(Decoded("?a=1&copy=2", TextMode::kAttributeValue), "?a=1&copy=2");
  EXPECT_EQ(Decoded("?a=1&copyright", TextMode::kAttributeValue), "?a=1&copyright");
  EXPECT_EQ(Decoded("?a=1&copy;=2", TextMode::kAttributeValue), "?a=1\xc2\xa9=2");
  EXPECT_EQ(Decoded("&copy 2026", TextMode::kAttributeValue), "\xc2\xa9 2026");
  EXPECT_EQ(Decoded("?a=1&copy=2"), "?a=1\xc2\xa9=2");
}

TEST(DecodeTest, NumericReferences) {
  EXPECT_EQ(Decoded("&#65;&#x42;&#X43"), "ABC");
  EXPECT_EQ(Decoded("&#128512;"), "\xf0\x9f\x98\x80");
  EXPECT_EQ(Decoded("&#x80;"), "\xe2\x82\xac");
  EXPECT_EQ(Decoded("&#0;&#xD800;&#x110000;&#99999999999;"), "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd");
  EXPECT_EQ(Decoded("&#;&#x;&#"), "&#;&#x;&#");
}

TEST(DecodeTest, NewlinesAreNormalised) {
  EXPECT_EQ(Decoded("a\r\nb\rc\n\rd"), "a\nb\nc\n\nd");
  EXPECT_EQ(Decoded("a\r\n&amp;", TextMode::kRawText), "a\n&amp;");
  EXPECT_EQ(Decoded("\r"), "\n");
}

TEST(DecodeTest, NeedsDecodingOnlyWhenSomethingChanges) {
  EXPECT_FALSE(NeedsDecoding("plain text", TextMode::kText));
  EXPECT_TRUE(NeedsDecoding("a &amp; b", TextMode::kText));
  EXPECT_TRUE(NeedsDecoding("a\r\nb", TextMode::kRawText));
  EXPECT_FALSE(NeedsDecoding("a &amp; b", TextMode::kRawText));
}

TEST(DecodeTest, SizeBoundCoversTheWidestReference) {
  EXPECT_GE(DecodedSizeBound(5), 6U);
  EXPECT_LE(DecodedSizeBound(1000), 1201U);
}

}  // anonymous namespace
}  // namespace arboris
