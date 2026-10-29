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

TEST(DOMQueryTest, TextConditionIsChecked) {
  const DOMManager dom("<p id=a>hello</p><p id=b>wor<b>ld</b></p><div id=c>world</div>");
  EXPECT_EQ(Ids(dom.FindAll({.tag = Tag::kP, .text = TextQueryCondition("world")})),
            std::vector<std::string_view>{"b"});
  EXPECT_EQ(Ids(dom.FindAll({.text = TextQueryCondition("world")})), (std::vector<std::string_view>{"b", "c"}));
  EXPECT_EQ(Ids(dom.FindAll({.text = TextQueryCondition([](std::string_view text) { return text.starts_with("h"); })})),
            std::vector<std::string_view>{"a"});
  EXPECT_FALSE(dom.Find({.tag = Tag::kP, .text = TextQueryCondition("nowhere")}).has_value());
}

TEST(DOMQueryTest, NoConditionMatchesEveryElement) {
  const DOMManager dom("<div id=a><p id=b></p></div><span id=c></span>");
  EXPECT_EQ(Ids(dom.FindAll({})), (std::vector<std::string_view>{"a", "b", "c"}));

  const auto outer = dom.Find({.tag = Tag::kDiv});
  ASSERT_TRUE(outer.has_value());
  EXPECT_EQ(Ids(outer->FindAll({})), std::vector<std::string_view>{"b"});
}

TEST(DOMQueryTest, ConditionNothingHasReturnsNothing) {
  const DOMManager dom("<div class=x id=a><div class=y id=b></div></div>");
  EXPECT_TRUE(dom.FindAll({.tag = Tag::kDiv, .classes = ClassSet{"no-such-class"}}).empty());
  EXPECT_TRUE(dom.FindAll({.tag = Tag::kDiv, .attributes = AttributeMap{{"no-such-attr", {}}}}).empty());

  const auto outer = dom.Find({.tag = Tag::kDiv});
  ASSERT_TRUE(outer.has_value());
  EXPECT_TRUE(outer->FindAll({.tag = Tag::kDiv, .classes = ClassSet{"x"}}).empty());
}

TEST(DOMQueryTest, ManagerFindsById) {
  const DOMManager dom("<div id=a><p id=b></p></div>");
  const auto found = dom.Find(std::string("b"));
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(found->Get().tag(), Tag::kP);
  EXPECT_FALSE(dom.Find(std::string("c")).has_value());
}

TEST(DOMQueryTest, TagAndAttributeNamesIgnoreCase) {
  const DOMManager dom("<DIV id=a><A HREF=\"/x\" id=b>x</A></DIV><Div id=c></dIV>");
  EXPECT_EQ(dom.parse_error_offset(), std::nullopt);
  EXPECT_EQ(Ids(dom.FindAll({.tag = Tag::kDiv})), (std::vector<std::string_view>{"a", "c"}));
  EXPECT_EQ(Ids(dom.FindAll({.attributes = AttributeMap{{"href", {"/x"}}}})), std::vector<std::string_view>{"b"});

  const auto link = dom.Find({.tag = Tag::kA});
  ASSERT_TRUE(link.has_value());
  EXPECT_EQ(link->Get().name(), "a");
  EXPECT_EQ(link->Get().attributes()[0].name, "href");
  EXPECT_EQ(link->Get().attributes()[0].value, "/x");
}

TEST(DOMQueryTest, UnknownTagsKeepTheirName) {
  const DOMManager dom("<my-card id=a><svg id=s><Path id=b /></svg><my-card id=c></my-card></my-card>");
  EXPECT_EQ(dom.parse_error_offset(), std::nullopt);

  const auto cards = dom.FindAll({.name = "my-card"});
  EXPECT_EQ(Ids(cards), (std::vector<std::string_view>{"a", "c"}));
  EXPECT_EQ(cards[0].Get().tag(), Tag::kUnknown);
  EXPECT_EQ(cards[0].Get().name(), "my-card");
  EXPECT_EQ(Ids(cards[0].FindAll({.name = "my-card"})), std::vector<std::string_view>{"c"});

  EXPECT_EQ(Ids(dom.FindAll({.name = "path"})), std::vector<std::string_view>{"b"});
  EXPECT_EQ(Ids(dom.FindAll({.name = "svg"})), std::vector<std::string_view>{"s"});
  EXPECT_TRUE(dom.FindAll({.name = "no-such"}).empty());
}

TEST(DOMQueryTest, UnknownEndTagsMatchByName) {
  const DOMManager dom("<x-a id=a></x-b><span id=b>y</span></x-a>");
  EXPECT_EQ(dom.parse_error_offset(), std::nullopt);
  const auto span = dom.Find({.tag = Tag::kSpan});
  ASSERT_TRUE(span.has_value());
  EXPECT_EQ(span->Get().parent().name(), "x-a");

  const DOMManager misnested("<x-a><x-b></x-a>");
  EXPECT_TRUE(misnested.parse_error_offset().has_value());
}

TEST(DOMQueryTest, RepeatedNamesAreKeptOnceHoweverMany) {
  std::string classes;
  std::string attributes;
  for (int i = 0; i < 10000; ++i) {
    classes += " c" + std::to_string(i % 5000);
    attributes += " a" + std::to_string(i % 5000) + "=" + std::to_string(i);
  }
  const DOMManager dom("<div class=\"" + classes + "\"" + attributes + "></div>");
  const auto div = dom.Find({.tag = Tag::kDiv});
  ASSERT_TRUE(div.has_value());
  EXPECT_EQ(div->Get().classes().size(), 5000U);
  EXPECT_EQ(div->Get().attributes().size(), 5001U);
  EXPECT_EQ(dom.FindAll({.classes = ClassSet{"c4999"}}).size(), 1U);
  EXPECT_EQ(Ids(dom.FindAll({.attributes = AttributeMap{{"a0", {"0"}}}})), std::vector<std::string_view>{""});
  EXPECT_TRUE(dom.FindAll({.attributes = AttributeMap{{"a0", {"5000"}}}}).empty());
}

}  // anonymous namespace
}  // namespace arboris
