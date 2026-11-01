/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "dom/dom_manager.hpp"
#include "dom/node_ref.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

NodeRef ById(const DOMManager& dom, const char* id) {
  const auto found = dom.Find(std::string(id));
  EXPECT_TRUE(found.has_value()) << id;
  return found->Get();
}

TEST(SerializationTest, OuterAndInnerHtmlAreSourceSlices) {
  const DOMManager dom("<div id=d class=\"x  y\">a &amp; <b id=b>c</b>\n</div>");
  EXPECT_EQ(ById(dom, "d").outer_html(), "<div id=d class=\"x  y\">a &amp; <b id=b>c</b>\n</div>");
  EXPECT_EQ(ById(dom, "d").inner_html(), "a &amp; <b id=b>c</b>\n");
  EXPECT_EQ(ById(dom, "b").outer_html(), "<b id=b>c</b>");
  EXPECT_EQ(ById(dom, "b").inner_html(), "c");
  EXPECT_EQ(ById(dom, "d").text_content(), "a & c\n");
}

TEST(SerializationTest, TheRootIsTheWholeDocument) {
  const DOMManager dom("<!DOCTYPE html>\n<p>x</p>\n");
  EXPECT_EQ(dom.GetRoot().outer_html(), "<!DOCTYPE html>\n<p>x</p>\n");
  EXPECT_EQ(dom.GetRoot().inner_html(), "<!DOCTYPE html>\n<p>x</p>\n");
}

TEST(SerializationTest, ElementsWithoutAnEndTagEndWhereTheyWereClosed) {
  const DOMManager dom("<ul id=u><li id=a>one<li id=b>two</ul><br id=r><svg id=s><path id=\"p\"/></svg>");
  EXPECT_EQ(ById(dom, "a").outer_html(), "<li id=a>one");
  EXPECT_EQ(ById(dom, "a").inner_html(), "one");
  EXPECT_EQ(ById(dom, "b").outer_html(), "<li id=b>two");
  EXPECT_EQ(ById(dom, "u").inner_html(), "<li id=a>one<li id=b>two");
  EXPECT_EQ(ById(dom, "r").outer_html(), "<br id=r>");
  EXPECT_EQ(ById(dom, "r").inner_html(), "");
  EXPECT_EQ(ById(dom, "p").outer_html(), "<path id=\"p\"/>");
  EXPECT_EQ(ById(dom, "s").inner_html(), "<path id=\"p\"/>");
}

TEST(SerializationTest, ElementsLeftOpenRunToTheEndOfTheDocument) {
  const DOMManager dom("<div id=d><p id=p>x");
  EXPECT_EQ(ById(dom, "d").outer_html(), "<div id=d><p id=p>x");
  EXPECT_EQ(ById(dom, "p").inner_html(), "x");
}

TEST(SerializationTest, StrayEndTagsStayInTheParentsContents) {
  const DOMManager dom("<div id=d><p id=p>x</p></span>y</div>");
  EXPECT_EQ(ById(dom, "p").outer_html(), "<p id=p>x</p>");
  EXPECT_EQ(ById(dom, "d").inner_html(), "<p id=p>x</p></span>y");
}

TEST(SerializationTest, RawTextIsKeptAsWritten) {
  const DOMManager dom("<script id=s>if (a < b) { s = \"</p>\"; }</script>");
  EXPECT_EQ(ById(dom, "s").inner_html(), "if (a < b) { s = \"</p>\"; }");
  EXPECT_EQ(ById(dom, "s").text_content(), "if (a < b) { s = \"</p>\"; }");
}

}  // anonymous namespace
}  // namespace arboris
