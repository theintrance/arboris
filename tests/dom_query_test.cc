/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <vector>

#include "dom/dom_manager.hpp"
#include "dom/dom_query.hpp"
#include "utils/html_tokens.hpp"
#include "utils/query_options.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

std::vector<std::string_view> Ids(const std::vector<DOMQuery>& queries) {
  std::vector<std::string_view> out;
  for (const auto& query : queries) {
    out.push_back(query.Get().id());
  }
  return out;
}

TEST(DOMQueryTest, RootIsNotAnElement) {
  const DOMManager dom("<html><body><p>x</p></body></html>");
  EXPECT_EQ(dom.GetRoot().tag(), Tag::kDocument);

  const auto html = dom.FindAll({.tag = Tag::kHtml});
  ASSERT_EQ(html.size(), 1U);
  EXPECT_NE(html[0].Get().key(), dom.GetRoot().key());
  EXPECT_EQ(html[0].Get().parent().key(), dom.GetRoot().key());

  EXPECT_TRUE(dom.FindAll({.tag = Tag::kDocument}).empty());
}

TEST(DOMQueryTest, TopLevelQuerySearchesTheWholeDocument) {
  const DOMManager dom("<div id=a><div id=b></div></div><div id=c></div>");
  EXPECT_EQ(Ids(dom.FindAll({.tag = Tag::kDiv})), (std::vector<std::string_view>{"a", "b", "c"}));
}

TEST(DOMQueryTest, SubqueryFindsDescendantsOnly) {
  const DOMManager dom("<div id=a class=box><div id=b class=box></div></div>");
  const auto outer = dom.Find({.tag = Tag::kDiv});
  ASSERT_TRUE(outer.has_value());
  EXPECT_EQ(outer->Get().id(), "a");

  EXPECT_EQ(Ids(outer->FindAll({.tag = Tag::kDiv})), std::vector<std::string_view>{"b"});
  EXPECT_EQ(Ids(outer->FindAll({.classes = ClassSet{"box"}})), std::vector<std::string_view>{"b"});
  EXPECT_EQ(Ids(outer->FindAll({.attributes = AttributeMap{{"id", {}}}})), std::vector<std::string_view>{"b"});

  const auto inner = outer->Find({.tag = Tag::kDiv});
  ASSERT_TRUE(inner.has_value());
  EXPECT_EQ(inner->Get().id(), "b");
  EXPECT_TRUE(inner->FindAll({.tag = Tag::kDiv}).empty());
}

TEST(DOMQueryTest, FindByIdSearchesDescendantsOnly) {
  const DOMManager dom("<div id=a><div id=b></div></div>");
  const auto outer = dom.Find({.tag = Tag::kDiv});
  ASSERT_TRUE(outer.has_value());

  EXPECT_FALSE(outer->Find(std::string("a")).has_value());
  const auto inner = outer->Find(std::string("b"));
  ASSERT_TRUE(inner.has_value());
  EXPECT_EQ(inner->Get().id(), "b");
}

}  // anonymous namespace
}  // namespace arboris
