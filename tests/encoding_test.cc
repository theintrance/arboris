/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <string_view>

#include "dom/dom_manager.hpp"
#include "encoding/encoding.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

constexpr std::string_view kHangulUtf8 = "\xed\x95\x9c\xea\xb8\x80";
constexpr std::string_view kHangulEucKr = "\xc7\xd1\xb1\xdb";

std::string_view TextOf(const DOMManager& dom, Tag tag) {
  const auto found = dom.Find({.tag = tag});
  return found ? found->Get().text_content() : std::string_view{};
}

TEST(EncodingTest, LabelsResolveToNames) {
  EXPECT_EQ(CanonicalEncoding("UTF8"), "utf-8");
  EXPECT_EQ(CanonicalEncoding(" latin1 "), "windows-1252");
  EXPECT_EQ(CanonicalEncoding("ks_c_5601-1987"), "euc-kr");
  EXPECT_EQ(CanonicalEncoding("no-such-encoding"), std::nullopt);
}

TEST(EncodingTest, SniffsBomAndMeta) {
  EXPECT_EQ(SniffEncoding("\xef\xbb\xbf<p>"), "utf-8");
  EXPECT_EQ(SniffEncoding("\xff\xfe<\0p\0"), "utf-16le");
  EXPECT_EQ(SniffEncoding("<html><head><meta charset=EUC-KR></head>"), "euc-kr");
  EXPECT_EQ(SniffEncoding("<META CHARSET=\"utf-8\">"), "utf-8");
  EXPECT_EQ(SniffEncoding("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\">"),
            "windows-1252");
  EXPECT_EQ(SniffEncoding("<meta content=\"text/html; charset=euc-kr\" http-equiv=content-type>"), "euc-kr");
  EXPECT_EQ(SniffEncoding("<!-- <meta charset=euc-kr> --><p>"), std::nullopt);
  EXPECT_EQ(SniffEncoding("<meta name=viewport content=width><p>"), std::nullopt);
  EXPECT_EQ(SniffEncoding(std::string(1024, ' ') + "<meta charset=euc-kr>"), std::nullopt);
}

TEST(EncodingTest, Utf16IsConvertedInHouse) {
  EXPECT_EQ(ToUtf8(std::string_view("<\0p\0>\0", 6), "utf-16le"), "<p>");
  EXPECT_EQ(ToUtf8(std::string_view("\0<\0p\0>", 6), "utf-16be"), "<p>");
  EXPECT_EQ(ToUtf8(std::string_view("\x3d\xd8\x00\xde", 4), "utf-16le"), "\xf0\x9f\x98\x80");
  EXPECT_EQ(ToUtf8(std::string_view("\x3d\xd8" "a\0", 4), "utf-16le"), "\xef\xbf\xbd" "a");
  EXPECT_EQ(ToUtf8(std::string_view("a\0" "b", 3), "utf-16le"), "a\xef\xbf\xbd");
}

TEST(EncodingTest, DocumentFollowsBomThenCallerThenMeta) {
  const DOMManager bom(std::string("\xef\xbb\xbf<p>") + std::string(kHangulUtf8) + "</p>");
  EXPECT_EQ(bom.encoding(), "utf-8");
  EXPECT_EQ(TextOf(bom, Tag::kP), kHangulUtf8);

  const DOMManager utf16(std::string("\xff\xfe<\0p\0>\0", 8) + "\\\0" "\0p\0>\0", "euc-kr");
  EXPECT_EQ(utf16.encoding(), "utf-16le");

  const DOMManager plain(std::string("<p>") + std::string(kHangulUtf8) + "</p>");
  EXPECT_EQ(plain.encoding(), "utf-8");
  EXPECT_EQ(plain.unsupported_encoding(), std::nullopt);
  EXPECT_EQ(TextOf(plain, Tag::kP), kHangulUtf8);

  const DOMManager unknown("<p>x</p>", "x-no-such");
  EXPECT_EQ(unknown.encoding(), "utf-8");
  EXPECT_EQ(unknown.unsupported_encoding(), "x-no-such");
  EXPECT_EQ(TextOf(unknown, Tag::kP), "x");
}

TEST(EncodingTest, LegacyEncodingsGoThroughIconv) {
  const std::string euc_kr = "<meta charset=euc-kr><p>" + std::string(kHangulEucKr) + "</p>";
  const DOMManager sniffed(euc_kr);
  const DOMManager told("<p>" + std::string(kHangulEucKr) + "</p>", "ks_c_5601-1987");
  const DOMManager latin("<p>caf\xe9</p>", "latin1");
  if (sniffed.unsupported_encoding()) {
    GTEST_SKIP() << "built without iconv";
  }
  EXPECT_EQ(sniffed.encoding(), "euc-kr");
  EXPECT_EQ(TextOf(sniffed, Tag::kP), kHangulUtf8);
  EXPECT_EQ(told.encoding(), "euc-kr");
  EXPECT_EQ(TextOf(told, Tag::kP), kHangulUtf8);
  EXPECT_EQ(latin.encoding(), "windows-1252");
  EXPECT_EQ(TextOf(latin, Tag::kP), "caf\xc3\xa9");
}

}  // anonymous namespace
}  // namespace arboris
