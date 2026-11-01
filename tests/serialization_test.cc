/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <string_view>

#include "arboris/dom/dom_manager.hpp"
#include "arboris/dom/node_ref.hpp"
#include "arboris/utils/tag.hpp"

namespace arboris {
namespace {

// A value-returning helper cannot ASSERT, so the caller does.
std::optional<NodeRef> ById(const DOMManager& dom, const char* id) {
  const auto found = dom.Find(std::string(id));
  if (!found.has_value()) {
    return std::nullopt;
  }
  return found->Get();
}

TEST(SerializationTest, OuterAndInnerHtmlAreSourceSlices) {
  const DOMManager dom("<div id=d class=\"x  y\">a &amp; <b id=b>c</b>\n</div>");
  const auto d = ById(dom, "d");
  const auto b = ById(dom, "b");
  ASSERT_TRUE(d.has_value());
  ASSERT_TRUE(b.has_value());
  EXPECT_EQ(d->outer_html(), "<div id=d class=\"x  y\">a &amp; <b id=b>c</b>\n</div>");
  EXPECT_EQ(d->inner_html(), "a &amp; <b id=b>c</b>\n");
  EXPECT_EQ(b->outer_html(), "<b id=b>c</b>");
  EXPECT_EQ(b->inner_html(), "c");
  EXPECT_EQ(d->text_content(), "a & c\n");
}

TEST(SerializationTest, TheRootIsTheWholeDocument) {
  const DOMManager dom("<!DOCTYPE html>\n<p>x</p>\n");
  EXPECT_EQ(dom.GetRoot().outer_html(), "<!DOCTYPE html>\n<p>x</p>\n");
  EXPECT_EQ(dom.GetRoot().inner_html(), "<!DOCTYPE html>\n<p>x</p>\n");
}

TEST(SerializationTest, ElementsWithoutAnEndTagEndWhereTheyWereClosed) {
  const DOMManager dom("<ul id=u><li id=a>one<li id=b>two</ul><br id=r><svg id=s><path id=\"p\"/></svg>");
  const auto u = ById(dom, "u");
  const auto a = ById(dom, "a");
  const auto b = ById(dom, "b");
  const auto r = ById(dom, "r");
  const auto s = ById(dom, "s");
  const auto p = ById(dom, "p");
  ASSERT_TRUE(u.has_value());
  ASSERT_TRUE(a.has_value());
  ASSERT_TRUE(b.has_value());
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(s.has_value());
  ASSERT_TRUE(p.has_value());
  EXPECT_EQ(a->outer_html(), "<li id=a>one");
  EXPECT_EQ(a->inner_html(), "one");
  EXPECT_EQ(b->outer_html(), "<li id=b>two");
  EXPECT_EQ(u->inner_html(), "<li id=a>one<li id=b>two");
  EXPECT_EQ(r->outer_html(), "<br id=r>");
  EXPECT_EQ(r->inner_html(), "");
  EXPECT_EQ(p->outer_html(), "<path id=\"p\"/>");
  EXPECT_EQ(s->inner_html(), "<path id=\"p\"/>");
}

TEST(SerializationTest, ElementsLeftOpenRunToTheEndOfTheDocument) {
  const DOMManager dom("<div id=d><p id=p>x");
  const auto d = ById(dom, "d");
  const auto p = ById(dom, "p");
  ASSERT_TRUE(d.has_value());
  ASSERT_TRUE(p.has_value());
  EXPECT_EQ(d->outer_html(), "<div id=d><p id=p>x");
  EXPECT_EQ(p->inner_html(), "x");
}

TEST(SerializationTest, StrayEndTagsStayInTheParentsContents) {
  const DOMManager dom("<div id=d><p id=p>x</p></span>y</div>");
  const auto d = ById(dom, "d");
  const auto p = ById(dom, "p");
  ASSERT_TRUE(d.has_value());
  ASSERT_TRUE(p.has_value());
  EXPECT_EQ(p->outer_html(), "<p id=p>x</p>");
  EXPECT_EQ(d->inner_html(), "<p id=p>x</p></span>y");
}

TEST(SerializationTest, RawTextIsKeptAsWritten) {
  const DOMManager dom("<script id=s>if (a < b) { s = \"</p>\"; }</script>");
  const auto s = ById(dom, "s");
  ASSERT_TRUE(s.has_value());
  EXPECT_EQ(s->inner_html(), "if (a < b) { s = \"</p>\"; }");
  EXPECT_EQ(s->text_content(), "if (a < b) { s = \"</p>\"; }");
}

}  // anonymous namespace
}  // namespace arboris
