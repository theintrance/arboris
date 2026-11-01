/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "arboris/dom/dom_manager.hpp"
#include "arboris/dom/node_ref.hpp"
#include "arboris/utils/tag.hpp"

namespace arboris {
namespace {

std::vector<std::string_view> Ids(const auto& nodes) {
  std::vector<std::string_view> out;
  for (const NodeRef node : nodes) {
    out.push_back(node.id());
  }
  return out;
}

using Expected = std::vector<std::string_view>;

static_assert(std::forward_iterator<ChildRange::iterator>);
static_assert(std::forward_iterator<AncestorRange::iterator>);

TEST(NodeRefTest, SiblingsAreWalkedWithoutAllocation) {
  const DOMManager dom("<ul><li id=a><b><i></i></b><b></b></li><li id=b></li><li id=c></li></ul>");
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
  EXPECT_EQ(dom.GetRoot().first_child()->tag(), Tag::kUl);
  EXPECT_FALSE(b->Get().first_child().has_value());
  EXPECT_TRUE(b->Get().previous_sibling()->is_first_child());
  EXPECT_FALSE(b->Get().is_first_child());
  EXPECT_FALSE(dom.GetRoot().is_first_child());
}

TEST(NodeRefTest, ChildrenAreARangeOverTheSubtree) {
  const DOMManager dom("<div id=d><p id=a><b id=b></b></p><p id=c></p>text<br id=e></div><div id=f></div>");
  const auto div = dom.Find(std::string("d"));
  ASSERT_TRUE(div.has_value());
  EXPECT_EQ(Ids(div->Get().children()), (Expected{"a", "c", "e"}));
  EXPECT_EQ(Ids(dom.GetRoot().children()), (Expected{"d", "f"}));
  EXPECT_TRUE(dom.Find(std::string("b"))->Get().children().empty());
  EXPECT_TRUE(dom.Find(std::string("e"))->Get().children().empty());
  EXPECT_EQ(std::ranges::distance(div->Get().children()), 3);
  EXPECT_EQ(div->Get().children().begin()->id(), "a");
  EXPECT_EQ(std::next(div->Get().children().begin(), 3), div->Get().children().end());
}

TEST(NodeRefTest, AncestorsStopBelowTheRoot) {
  const DOMManager dom("<html id=h><body id=b><div id=d><p id=p></p></div></body></html>");
  const auto p = dom.Find(std::string("p"));
  ASSERT_TRUE(p.has_value());
  EXPECT_EQ(Ids(p->Get().ancestors()), (Expected{"d", "b", "h"}));
  EXPECT_TRUE(dom.Find(std::string("h"))->Get().ancestors().empty());
  EXPECT_TRUE(dom.GetRoot().ancestors().empty());
}

}  // anonymous namespace
}  // namespace arboris
