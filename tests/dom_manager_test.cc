/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dom/dom_manager.hpp"
#include "dom/node_ref.hpp"
#include "utils/query_options.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

constexpr std::string_view kTail = "<span>y</span>";

std::vector<NodeRef> FindAllByTag(const DOMManager& dom, Tag tag) {
  std::vector<NodeRef> out;
  for (const auto& query : dom.FindAll({.tag = tag})) {
    out.push_back(query.Get());
  }
  return out;
}

std::vector<Tag> ChildTags(const NodeRef& node) {
  std::vector<Tag> out;
  for (const auto& child : node.children()) {
    out.push_back(child.tag());
  }
  return out;
}

void ExpectReadsToTheEnd(std::string_view html) {
  SCOPED_TRACE(html);
  const DOMManager dom(std::string(html) + std::string(kTail));
  EXPECT_EQ(dom.parse_error_offset(), std::nullopt);

  const auto spans = FindAllByTag(dom, Tag::kSpan);
  ASSERT_EQ(spans.size(), 1U);
  EXPECT_EQ(spans[0].parent().key(), dom.GetRoot().key());
}

TEST(DOMManagerParseTest, CommentsAndDoctypeMakeNoElement) {
  ExpectReadsToTheEnd("<div><!-- note --><p>x</p></div>");
  ExpectReadsToTheEnd("<!DOCTYPE html><div></div>");
  ExpectReadsToTheEnd("<?xml version=\"1.0\"?><div></div>");
  ExpectReadsToTheEnd("<div><![endif]></div>");

  const DOMManager dom("<!DOCTYPE html><html><body><!-- a > b --><p id=x>t</p></body></html>");
  EXPECT_EQ(ChildTags(dom.GetRoot()), std::vector<Tag>{Tag::kHtml});

  const auto body = FindAllByTag(dom, Tag::kBody);
  ASSERT_EQ(body.size(), 1U);
  EXPECT_TRUE(body[0].text_runs().empty());
}

TEST(DOMManagerParseTest, CommentedOutMarkupIsNotInTheDom) {
  const DOMManager dom("<!-- <div id=\"main\"> --><div id=\"main\"></div>");
  const auto main = dom.Find({.attributes = AttributeMap{{"id", {"main"}}}});
  ASSERT_TRUE(main.has_value());
  EXPECT_EQ(main->Get().tag(), Tag::kDiv);
  EXPECT_EQ(FindAllByTag(dom, Tag::kDiv).size(), 1U);
}

TEST(DOMManagerParseTest, ScriptAndStyleHoldRawText) {
  ExpectReadsToTheEnd("<div><script>if (a < b) { s = 1; }</script></div>");
  ExpectReadsToTheEnd("<div><script>s = \"</p>\";</script></div>");
  ExpectReadsToTheEnd("<div><style>a > b { color: red; }</style></div>");
  ExpectReadsToTheEnd("<div><script>x = 1;</SCRIPT ></div>");

  const DOMManager dom("<script>if (a < b) { s = \"<div>\"; }</script>");
  EXPECT_TRUE(FindAllByTag(dom, Tag::kDiv).empty());
  const auto script = FindAllByTag(dom, Tag::kScript);
  ASSERT_EQ(script.size(), 1U);
  EXPECT_EQ(script[0].text_content(), "if (a < b) { s = \"<div>\"; }");
  EXPECT_EQ(script[0].sub_tree_size(), 1U);
}

TEST(DOMManagerParseTest, TextareaAndTitleHoldText) {
  ExpectReadsToTheEnd("<textarea>a<b</textarea>");
  ExpectReadsToTheEnd("<title>a <b> c</title>");

  const DOMManager dom("<textarea>a<b</textarea>");
  const auto textarea = FindAllByTag(dom, Tag::kTextarea);
  ASSERT_EQ(textarea.size(), 1U);
  EXPECT_EQ(textarea[0].text_content(), "a<b");
  EXPECT_TRUE(textarea[0].children().empty());
}

TEST(DOMManagerParseTest, OmittedEndTagsAreImplied) {
  ExpectReadsToTheEnd("<ul><li>a<li>b</ul>");
  ExpectReadsToTheEnd("<div><p>a<p>b</div>");
  ExpectReadsToTheEnd("<p>a<div>b</div>");
  ExpectReadsToTheEnd("<dl><dt>a<dd>b<dt>c</dl>");
  ExpectReadsToTheEnd("<select><option>a<option>b<optgroup><option>c</select>");
  ExpectReadsToTheEnd("<table><tr><td>a<td>b<tr><td>c</table>");
  ExpectReadsToTheEnd("<table><thead><tr><th>a<tbody><tr><td>b</table>");
  ExpectReadsToTheEnd("<html><head><title>t</title><body><p>x</body></html>");

  const DOMManager dom("<ul><li>a<li>b<ul><li>c</ul><li>d</ul>");
  const auto lists = FindAllByTag(dom, Tag::kUl);
  ASSERT_EQ(lists.size(), 2U);
  EXPECT_EQ(ChildTags(lists[0]), (std::vector<Tag>{Tag::kLi, Tag::kLi, Tag::kLi}));
  EXPECT_EQ(ChildTags(lists[1]), std::vector<Tag>{Tag::kLi});

  const DOMManager table("<table><tr><td>a<td>b<tr><td>c</table>");
  const auto rows = FindAllByTag(table, Tag::kTr);
  ASSERT_EQ(rows.size(), 2U);
  EXPECT_EQ(rows[0].parent().tag(), Tag::kTable);
  EXPECT_EQ(rows[1].parent().tag(), Tag::kTable);
  EXPECT_EQ(ChildTags(rows[0]), (std::vector<Tag>{Tag::kTd, Tag::kTd}));
  EXPECT_EQ(rows[0].children()[1].text_content(), "b");

  const DOMManager page("<html><head><title>t</title><body><p>x</body></html>");
  const auto body = FindAllByTag(page, Tag::kBody);
  ASSERT_EQ(body.size(), 1U);
  EXPECT_EQ(body[0].parent().tag(), Tag::kHtml);
  EXPECT_EQ(ChildTags(body[0].parent()), (std::vector<Tag>{Tag::kHead, Tag::kBody}));
}

TEST(DOMManagerParseTest, EndTagForElementNotOpenIsDropped) {
  ExpectReadsToTheEnd("<p><div>x</div></p>");
  ExpectReadsToTheEnd("<div><img src=a></img><br></br></div>");
}

TEST(DOMManagerParseTest, SelfClosingClosesInsideSvgAndMath) {
  ExpectReadsToTheEnd("<div><svg><path d=\"M0 0\"/></svg></div>");
  ExpectReadsToTheEnd("<div><svg viewBox=\"0 0 1 1\"><g><circle r=1 /><rect/></g></svg></div>");
  ExpectReadsToTheEnd("<div><math><mi>x</mi><mspace/></math></div>");
  ExpectReadsToTheEnd("<div><svg/></div>");

  const DOMManager dom("<svg><path/><path/></svg>");
  const auto svg = FindAllByTag(dom, Tag::kSvg);
  ASSERT_EQ(svg.size(), 1U);
  EXPECT_EQ(ChildTags(svg[0]), (std::vector<Tag>{Tag::kUnknown, Tag::kUnknown}));
}

TEST(DOMManagerParseTest, SelfClosingHtmlElementStaysOpen) {
  const DOMManager dom("<div/><span>y</span></div>");
  EXPECT_EQ(dom.parse_error_offset(), std::nullopt);
  const auto spans = FindAllByTag(dom, Tag::kSpan);
  ASSERT_EQ(spans.size(), 1U);
  EXPECT_EQ(spans[0].parent().tag(), Tag::kDiv);
}

TEST(DOMManagerParseTest, ReportsWhereMisnestedMarkupStopsTheParse) {
  constexpr std::string_view kHtml = "<div><span>a</div><p>b</p>";
  const DOMManager dom(kHtml);
  ASSERT_TRUE(dom.parse_error_offset().has_value());
  EXPECT_EQ(*dom.parse_error_offset(), kHtml.find("</div>"));

  EXPECT_EQ(FindAllByTag(dom, Tag::kSpan).size(), 1U);
  EXPECT_TRUE(FindAllByTag(dom, Tag::kP).empty());
  for (const auto& node : FindAllByTag(dom, Tag::kDiv)) {
    EXPECT_GE(node.sub_tree_size(), 1U);
  }
}

TEST(DOMManagerParseTest, ReportsATagTheInputEndsIn) {
  constexpr std::string_view kHtml = "<div><a href=\"x";
  const DOMManager dom(kHtml);
  EXPECT_EQ(dom.parse_error_offset(), std::optional<std::size_t>(kHtml.find("<a")));
  EXPECT_EQ(FindAllByTag(dom, Tag::kDiv).size(), 1U);
}

}  // anonymous namespace
}  // namespace arboris
