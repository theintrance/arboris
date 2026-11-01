/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/dom_builder.hpp"
#include "dom/tag_node.hpp"
#include "utils/html_tokens.hpp"
#include "utils/tag.hpp"

namespace arboris {
namespace {

// Feeds the builder tokens by hand, as the tokenizer would, with text positions taken
// from one buffer so the text spans it hands back can be checked.
class DOMBuilderTest : public ::testing::Test {
 protected:
  bool Open(Tag tag, std::string_view name = {}, bool is_void = false, bool self_closing = false,
            std::span<const Attribute> attributes = {}) {
    HtmlToken token{{0, 0}, tag, is_void};
    token.is_self_closing = self_closing;
    token.name = name;
    return builder_.FeedOpenToken(token, attributes, cursor());
  }

  bool Close(Tag tag, std::string_view name = {}) {
    HtmlCloseToken token{{0, 0}, tag, name};
    return builder_.FeedCloseToken(token, cursor());
  }

  // Copies the text to where the cursor stands, as the string pool would.
  void Text(std::string_view text) {
    const auto begin = static_cast<std::uint32_t>(used_);
    std::ranges::copy(text, pool_.data() + used_);
    used_ += text.size();
    builder_.FeedTextToken(HtmlTextToken{{begin, begin + static_cast<std::uint32_t>(text.size())},
                                         std::string_view(pool_.data() + begin, text.size())});
  }

  [[nodiscard]] const char* cursor() const {
    return pool_.data() + used_;
  }

  [[nodiscard]] const TagNode& node(NodeKey key) const {
    return builder_.GetNodeList()[key];
  }

  DOMBuilder builder_;
  std::string pool_ = std::string(1024, '\0');
  std::size_t used_ = 0;
};

TEST_F(DOMBuilderTest, NodesAreKeyedInDfsOrderWithSubtreeSizes) {
  EXPECT_TRUE(Open(Tag::kDiv));
  EXPECT_TRUE(Open(Tag::kP));
  EXPECT_TRUE(Open(Tag::kSpan));
  EXPECT_TRUE(Close(Tag::kSpan));
  EXPECT_TRUE(Open(Tag::kA));
  EXPECT_TRUE(Close(Tag::kA));
  EXPECT_TRUE(Close(Tag::kP));
  EXPECT_TRUE(Close(Tag::kDiv));
  EXPECT_TRUE(builder_.Validate());
  builder_.Finish(cursor(), 0);

  ASSERT_EQ(builder_.GetNodeList().size(), 5U);
  EXPECT_EQ(node(0).tag(), Tag::kDocument);
  EXPECT_EQ(node(0).parent_key(), 0U);
  EXPECT_EQ(node(0).sub_tree_size(), 5U);
  EXPECT_EQ(node(1).tag(), Tag::kDiv);
  EXPECT_EQ(node(1).parent_key(), 0U);
  EXPECT_EQ(node(1).sub_tree_size(), 4U);
  EXPECT_EQ(node(2).tag(), Tag::kP);
  EXPECT_EQ(node(2).parent_key(), 1U);
  EXPECT_EQ(node(2).sub_tree_size(), 3U);
  EXPECT_EQ(node(3).tag(), Tag::kSpan);
  EXPECT_EQ(node(3).parent_key(), 2U);
  EXPECT_EQ(node(3).sub_tree_size(), 1U);
  EXPECT_EQ(node(4).tag(), Tag::kA);
  EXPECT_EQ(node(4).parent_key(), 2U);
}

TEST_F(DOMBuilderTest, VoidTagsAndForeignSelfClosingTagsCloseAtOnce) {
  EXPECT_TRUE(Open(Tag::kDiv));
  EXPECT_TRUE(Open(Tag::kBr, {}, true));
  EXPECT_TRUE(Open(Tag::kDiv, {}, false, true));  // <div/> is just <div> in HTML
  EXPECT_TRUE(Open(Tag::kSvg));
  EXPECT_TRUE(Open(Tag::kUnknown, "path", false, true));
  EXPECT_TRUE(Open(Tag::kUnknown, "rect"));
  EXPECT_TRUE(Close(Tag::kUnknown, "rect"));
  EXPECT_TRUE(Close(Tag::kSvg));
  EXPECT_FALSE(builder_.Validate());  // the inner <div> is still open
  EXPECT_TRUE(Close(Tag::kDiv));
  EXPECT_TRUE(Close(Tag::kDiv));
  EXPECT_TRUE(builder_.Validate());

  EXPECT_EQ(node(2).tag(), Tag::kBr);
  EXPECT_EQ(node(2).sub_tree_size(), 1U);
  EXPECT_EQ(node(3).parent_key(), 1U);
  EXPECT_EQ(node(4).tag(), Tag::kSvg);
  EXPECT_EQ(node(4).parent_key(), 3U);
  EXPECT_EQ(node(5).name(), "path");
  EXPECT_EQ(node(5).sub_tree_size(), 1U);
  EXPECT_EQ(node(6).name(), "rect");
  EXPECT_EQ(node(6).parent_key(), 4U);
}

TEST_F(DOMBuilderTest, StartTagsCloseElementsWithOptionalEndTags) {
  EXPECT_TRUE(Open(Tag::kUl));
  EXPECT_TRUE(Open(Tag::kLi));
  EXPECT_TRUE(Open(Tag::kLi));
  EXPECT_TRUE(Open(Tag::kP));
  EXPECT_TRUE(Open(Tag::kDiv));  // <div> ends an open <p>
  EXPECT_TRUE(Close(Tag::kDiv));
  EXPECT_TRUE(Close(Tag::kUl));  // closes the second <li> too
  EXPECT_TRUE(builder_.Validate());

  EXPECT_EQ(node(2).tag(), Tag::kLi);
  EXPECT_EQ(node(2).sub_tree_size(), 1U);
  EXPECT_EQ(node(3).tag(), Tag::kLi);
  EXPECT_EQ(node(3).parent_key(), 1U);
  EXPECT_EQ(node(4).tag(), Tag::kP);
  EXPECT_EQ(node(4).sub_tree_size(), 1U);
  EXPECT_EQ(node(5).tag(), Tag::kDiv);
  EXPECT_EQ(node(5).parent_key(), 3U);
}

TEST_F(DOMBuilderTest, StrayEndTagsAreDroppedAndMisnestedOnesFail) {
  EXPECT_TRUE(Open(Tag::kDiv));
  EXPECT_TRUE(Close(Tag::kSpan));  // nothing to close: dropped
  EXPECT_TRUE(Open(Tag::kBr, {}, true));
  EXPECT_TRUE(Close(Tag::kBr));  // </br> after a void tag: dropped
  EXPECT_TRUE(Open(Tag::kUnknown, "x-a"));
  EXPECT_TRUE(Close(Tag::kUnknown, "x-b"));  // another unknown tag: dropped
  EXPECT_TRUE(Close(Tag::kUnknown, "x-a"));
  EXPECT_TRUE(Open(Tag::kSpan));
  EXPECT_FALSE(Close(Tag::kDiv));  // would close the <span> the author left open
}

TEST_F(DOMBuilderTest, TextIsKeptPerTagAndAsOneSpanPerSubtree) {
  EXPECT_TRUE(Open(Tag::kP));
  Text("Hello");
  EXPECT_TRUE(Open(Tag::kB));
  Text("big");
  EXPECT_TRUE(Close(Tag::kB));
  Text("World");
  EXPECT_TRUE(Close(Tag::kP));
  builder_.Finish(cursor(), 0);

  EXPECT_EQ(node(1).text_content(), "HellobigWorld");
  EXPECT_EQ(node(2).text_content(), "big");

  const auto runs = builder_.ReleaseTextArena();
  ASSERT_EQ(runs.size(), 3U);
  EXPECT_EQ(node(2).text_count(), 1U);
  EXPECT_EQ(runs[node(2).text_begin()].text, "big");
  EXPECT_EQ(node(1).text_count(), 2U);
  EXPECT_EQ(runs[node(1).text_begin()].text, "Hello");
  EXPECT_EQ(runs[node(1).text_begin() + 1].text, "World");
  EXPECT_LT(runs[node(1).text_begin()].begin_pos, runs[node(1).text_begin() + 1].begin_pos);
}

TEST_F(DOMBuilderTest, AttributesIdAndClassesAreStoredPerNode) {
  const std::vector<Attribute> attributes{{"id", "main"}, {"class", " a  b a "}, {"href", "/x"}};
  EXPECT_TRUE(Open(Tag::kA, {}, false, false, attributes));
  EXPECT_TRUE(Close(Tag::kA));
  EXPECT_TRUE(Open(Tag::kA, {}, false, false, std::span(attributes).first(1)));
  EXPECT_TRUE(Close(Tag::kA));

  EXPECT_EQ(node(1).id(), "main");
  EXPECT_EQ(node(1).attr_count(), 3U);
  EXPECT_EQ(node(1).class_count(), 2U);
  EXPECT_EQ(node(2).attr_begin(), 3U);
  EXPECT_EQ(node(2).attr_count(), 1U);
  EXPECT_EQ(node(2).class_count(), 0U);

  const auto classes = builder_.ReleaseClassArena();
  ASSERT_EQ(classes.size(), 2U);
  EXPECT_EQ(classes[node(1).class_begin()], "a");
  EXPECT_EQ(classes[node(1).class_begin() + 1], "b");
  const auto stored = builder_.ReleaseAttrArena();
  ASSERT_EQ(stored.size(), 4U);
  EXPECT_EQ(stored[2].name, "href");
  EXPECT_EQ(stored[3].name, "id");
}

TEST_F(DOMBuilderTest, FinishClosesWhatWasLeftOpen) {
  EXPECT_TRUE(Open(Tag::kDiv));
  EXPECT_TRUE(Open(Tag::kSpan));
  Text("x");
  builder_.Finish(cursor(), 0);

  EXPECT_EQ(node(0).sub_tree_size(), 3U);
  EXPECT_EQ(node(1).sub_tree_size(), 2U);
  EXPECT_EQ(node(2).sub_tree_size(), 1U);
  EXPECT_EQ(node(2).text_content(), "x");
  EXPECT_EQ(node(1).text_content(), "x");
}

}  // anonymous namespace
}  // namespace arboris
