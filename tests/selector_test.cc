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
#include "selector/selector.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

std::string Page() {
  return "<html><body>"
         "<div id=main class=\"box wide\">"
         "  <p id=p1 class=intro>one</p>"
         "  <p id=p2>two <a id=a1 href=\"/docs/intro\" lang=\"en-US\" title=\"Hello World\">link</a></p>"
         "  <ul id=list><li id=l1>a<li id=l2 class=sel>b<li id=l3>c</ul>"
         "  <my-card id=card data-kind=\"x\"></my-card>"
         "</div>"
         "<div id=aside><p id=p3></p><a id=a2 href=\"https://example.com/x.pdf\">pdf</a></div>"
         "</body></html>";
}

std::vector<std::string_view> Ids(const std::vector<DOMQuery>& queries) {
  std::vector<std::string_view> out;
  for (const auto& query : queries) {
    out.push_back(query.Get().id());
  }
  return out;
}

using Expected = std::vector<std::string_view>;

TEST(SelectorTest, ParseRejectsWhatItDoesNotRead) {
  EXPECT_TRUE(Selector::Parse("div > a.link, #x[href^='/']:first-child").has_value());
  EXPECT_FALSE(Selector::Parse("").has_value());
  EXPECT_FALSE(Selector::Parse("div >").has_value());
  EXPECT_FALSE(Selector::Parse("[href=]").has_value());
  EXPECT_FALSE(Selector::Parse("a:hover").has_value());
  EXPECT_FALSE(Selector::Parse("a::before").has_value());
  EXPECT_FALSE(Selector::Parse("p:nth-child(x)").has_value());
  EXPECT_FALSE(Selector::Parse("div,").has_value());
  EXPECT_FALSE(Selector::Parse(".").has_value());
}

TEST(SelectorTest, SimpleSelectors) {
  const DOMManager dom(Page());
  EXPECT_EQ(Ids(dom.SelectAll("p")), (Expected{"p1", "p2", "p3"}));
  EXPECT_EQ(Ids(dom.SelectAll("P")), (Expected{"p1", "p2", "p3"}));
  EXPECT_EQ(Ids(dom.SelectAll("#p2")), Expected{"p2"});
  EXPECT_EQ(Ids(dom.SelectAll(".box.wide")), Expected{"main"});
  EXPECT_EQ(Ids(dom.SelectAll("p.intro")), Expected{"p1"});
  EXPECT_EQ(Ids(dom.SelectAll("my-card")), Expected{"card"});
  EXPECT_EQ(Ids(dom.SelectAll("*")).size(), 14U);
  EXPECT_TRUE(dom.SelectAll("span").empty());
  EXPECT_TRUE(dom.SelectAll("not a selector >").empty());
}

TEST(SelectorTest, AttributeOperators) {
  const DOMManager dom(Page());
  EXPECT_EQ(Ids(dom.SelectAll("[href]")), (Expected{"a1", "a2"}));
  EXPECT_EQ(Ids(dom.SelectAll("[HREF]")), (Expected{"a1", "a2"}));
  EXPECT_EQ(Ids(dom.SelectAll("[href='/docs/intro']")), Expected{"a1"});
  EXPECT_EQ(Ids(dom.SelectAll("[href=\"/docs/intro\"]")), Expected{"a1"});
  EXPECT_EQ(Ids(dom.SelectAll("a[href^='/']")), Expected{"a1"});
  EXPECT_EQ(Ids(dom.SelectAll("a[href$='.pdf']")), Expected{"a2"});
  EXPECT_EQ(Ids(dom.SelectAll("a[href*='example']")), Expected{"a2"});
  EXPECT_EQ(Ids(dom.SelectAll("[title~='World']")), Expected{"a1"});
  EXPECT_TRUE(dom.SelectAll("[title~='Wor']").empty());
  EXPECT_EQ(Ids(dom.SelectAll("[lang|='en']")), Expected{"a1"});
  EXPECT_TRUE(dom.SelectAll("[lang|='e']").empty());
  EXPECT_EQ(Ids(dom.SelectAll("[title='hello world' i]")), Expected{"a1"});
  EXPECT_TRUE(dom.SelectAll("[title='hello world']").empty());
  EXPECT_EQ(Ids(dom.SelectAll("[data-kind=x]")), Expected{"card"});
}

TEST(SelectorTest, Combinators) {
  const DOMManager dom(Page());
  EXPECT_EQ(Ids(dom.SelectAll("div a")), (Expected{"a1", "a2"}));
  EXPECT_EQ(Ids(dom.SelectAll("div > a")), Expected{"a2"});
  EXPECT_EQ(Ids(dom.SelectAll("#main > p")), (Expected{"p1", "p2"}));
  EXPECT_EQ(Ids(dom.SelectAll("body > div > p > a")), Expected{"a1"});
  EXPECT_EQ(Ids(dom.SelectAll("p + p")), Expected{"p2"});
  EXPECT_EQ(Ids(dom.SelectAll("#l1 + li")), Expected{"l2"});
  EXPECT_EQ(Ids(dom.SelectAll("#l1 ~ li")), (Expected{"l2", "l3"}));
  EXPECT_EQ(Ids(dom.SelectAll("p ~ ul")), Expected{"list"});
  EXPECT_EQ(Ids(dom.SelectAll("html body #main ul li.sel")), Expected{"l2"});
  EXPECT_TRUE(dom.SelectAll("ul > a").empty());
}

TEST(SelectorTest, PseudoClasses) {
  const DOMManager dom(Page());
  EXPECT_EQ(Ids(dom.SelectAll("li:first-child")), Expected{"l1"});
  EXPECT_EQ(Ids(dom.SelectAll("li:last-child")), Expected{"l3"});
  EXPECT_EQ(Ids(dom.SelectAll("p:only-child")), Expected{});
  EXPECT_EQ(Ids(dom.SelectAll("a:only-child")), Expected{"a1"});
  EXPECT_EQ(Ids(dom.SelectAll("li:nth-child(2)")), Expected{"l2"});
  EXPECT_EQ(Ids(dom.SelectAll("li:nth-child(odd)")), (Expected{"l1", "l3"}));
  EXPECT_EQ(Ids(dom.SelectAll("li:nth-child( 2n )")), Expected{"l2"});
  EXPECT_EQ(Ids(dom.SelectAll("li:nth-child(n+2)")), (Expected{"l2", "l3"}));
  EXPECT_EQ(Ids(dom.SelectAll("li:nth-child(-n+2)")), (Expected{"l1", "l2"}));
  EXPECT_EQ(Ids(dom.SelectAll("li:nth-last-child(1)")), Expected{"l3"});
  EXPECT_EQ(Ids(dom.SelectAll(":root")), Expected{""});
  EXPECT_EQ(dom.SelectAll(":root")[0].Get().tag(), Tag::kHtml);
  EXPECT_EQ(Ids(dom.SelectAll("p:empty")), Expected{"p3"});
  EXPECT_EQ(Ids(dom.SelectAll("my-card:empty")), Expected{"card"});
  EXPECT_EQ(Ids(dom.SelectAll("#main > p:not(.intro)")), Expected{"p2"});
  EXPECT_EQ(Ids(dom.SelectAll("li:not(:first-child):not(:last-child)")), Expected{"l2"});
}

TEST(SelectorTest, ListsMergeInDocumentOrder) {
  const DOMManager dom(Page());
  EXPECT_EQ(Ids(dom.SelectAll("a, p, #list")), (Expected{"p1", "p2", "a1", "list", "p3", "a2"}));
  EXPECT_EQ(Ids(dom.SelectAll("p, .intro, #p1")), (Expected{"p1", "p2", "p3"}));
}

TEST(SelectorTest, SelectReturnsTheFirstInDocumentOrder) {
  const DOMManager dom(Page());
  const auto first = dom.Select("a, p");
  ASSERT_TRUE(first.has_value());
  EXPECT_EQ(first->Get().id(), "p1");
  EXPECT_FALSE(dom.Select("span").has_value());
  EXPECT_FALSE(dom.Select("").has_value());
}

TEST(SelectorTest, SubqueriesScopeTheSubjectOnly) {
  const DOMManager dom(Page());
  const auto main = dom.Select("#main");
  ASSERT_TRUE(main.has_value());
  EXPECT_EQ(Ids(main->SelectAll("a")), Expected{"a1"});
  EXPECT_EQ(Ids(main->SelectAll("body > div > p")), (Expected{"p1", "p2"}));
  EXPECT_EQ(Ids(main->SelectAll("body a")), Expected{"a1"});
  EXPECT_TRUE(main->SelectAll("#main").empty());
  EXPECT_TRUE(main->SelectAll("#aside a").empty());
}

TEST(SelectorTest, ParsedSelectorsAreReusable) {
  const DOMManager dom(Page());
  const auto selector = Selector::Parse("li.sel");
  ASSERT_TRUE(selector.has_value());
  EXPECT_EQ(Ids(dom.SelectAll(*selector)), Expected{"l2"});
  EXPECT_TRUE(selector->Matches(dom.SelectAll(*selector)[0].Get()));
  EXPECT_FALSE(selector->Matches(dom.GetRoot()));
}

TEST(NodeRefTest, SiblingsAreWalkedWithoutAllocation) {
  const DOMManager dom("<ul><li id=a><b></b></li><li id=b></li><li id=c></li></ul>");
  const auto b = dom.Find(std::string("b"));
  ASSERT_TRUE(b.has_value());
  ASSERT_TRUE(b->Get().previous_sibling().has_value());
  EXPECT_EQ(b->Get().previous_sibling()->id(), "a");
  ASSERT_TRUE(b->Get().next_sibling().has_value());
  EXPECT_EQ(b->Get().next_sibling()->id(), "c");
  EXPECT_FALSE(b->Get().previous_sibling()->previous_sibling().has_value());
  EXPECT_FALSE(b->Get().next_sibling()->next_sibling().has_value());
  EXPECT_FALSE(dom.GetRoot().next_sibling().has_value());
  EXPECT_FALSE(dom.GetRoot().previous_sibling().has_value());
}

}  // anonymous namespace
}  // namespace arboris
