/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <memory>
#include <utility>
#include <vector>
#include <string>

#include "dom/dom_builder.hpp"
#include "dom/node.hpp"
#include "utils/html_tokens.hpp"
#include "utils/tag.hpp"

namespace arboris {

class DOMBuilderTest : public ::testing::Test {
 protected:
  DOMBuilder builder_;
  std::vector<std::shared_ptr<Node>> created_nodes_;

  void SetUp() override {
    created_nodes_.clear();
    builder_.SetNodeCreationCallback(
        [this](std::shared_ptr<Node> node) {
          created_nodes_.push_back(node);
        });
  }
};

// Test: Single element creation
// HTML Structure: <div></div>
TEST_F(DOMBuilderTest, SingleElement) {
  HtmlToken open_token;
  open_token.tag = Tag::kDiv;
  open_token.is_void_tag = false;

  EXPECT_TRUE(builder_.FeedOpenToken(std::move(open_token)));
  EXPECT_EQ(created_nodes_.size(), 1);
  EXPECT_EQ(created_nodes_[0]->id(), 0);
  EXPECT_EQ(created_nodes_[0]->html_token().tag, Tag::kDiv);
  EXPECT_EQ(created_nodes_[0]->in(), 1);

  HtmlCloseToken close_token;
  close_token.tag = Tag::kDiv;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(close_token)));
  EXPECT_EQ(created_nodes_[0]->out(), 2);
  EXPECT_TRUE(builder_.Validate());
}

// Test: Nested elements
// HTML Structure:
//   <div>
//     <p>
//       <span></span>
//     </p>
//   </div>
TEST_F(DOMBuilderTest, NestedElements) {
  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // <span>
  HtmlToken span_open;
  span_open.tag = Tag::kSpan;
  span_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(span_open)));

  // </span>
  HtmlCloseToken span_close;
  span_close.tag = Tag::kSpan;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(span_close)));

  // </p>
  HtmlCloseToken p_close;
  p_close.tag = Tag::kP;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(p_close)));

  // </div>
  HtmlCloseToken div_close;
  div_close.tag = Tag::kDiv;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(div_close)));

  // Verify 3 nodes were created
  EXPECT_EQ(created_nodes_.size(), 3);
  EXPECT_EQ(created_nodes_[0]->html_token().tag, Tag::kDiv);
  EXPECT_EQ(created_nodes_[1]->html_token().tag, Tag::kP);
  EXPECT_EQ(created_nodes_[2]->html_token().tag, Tag::kSpan);

  // Verify node IDs
  EXPECT_EQ(created_nodes_[0]->id(), 0);
  EXPECT_EQ(created_nodes_[1]->id(), 1);
  EXPECT_EQ(created_nodes_[2]->id(), 2);

  // Verify Euler tour timers
  EXPECT_EQ(created_nodes_[0]->in(), 1);
  EXPECT_EQ(created_nodes_[1]->in(), 2);
  EXPECT_EQ(created_nodes_[2]->in(), 3);
  EXPECT_EQ(created_nodes_[2]->out(), 4);
  EXPECT_EQ(created_nodes_[1]->out(), 5);
  EXPECT_EQ(created_nodes_[0]->out(), 6);

  EXPECT_TRUE(builder_.Validate());
}

// Test: Void tags (self-closing)
// HTML Structure:
//   <div>
//     <br>
//     <img>
//   </div>
TEST_F(DOMBuilderTest, VoidTag) {
  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <br> (void tag)
  HtmlToken br_token;
  br_token.tag = Tag::kBr;
  br_token.is_void_tag = true;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(br_token)));

  // <img> (void tag)
  HtmlToken img_token;
  img_token.tag = Tag::kImg;
  img_token.is_void_tag = true;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(img_token)));

  // </div>
  HtmlCloseToken div_close;
  div_close.tag = Tag::kDiv;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(div_close)));

  // Verify 3 nodes were created
  EXPECT_EQ(created_nodes_.size(), 3);
  EXPECT_EQ(created_nodes_[0]->html_token().tag, Tag::kDiv);
  EXPECT_EQ(created_nodes_[1]->html_token().tag, Tag::kBr);
  EXPECT_EQ(created_nodes_[2]->html_token().tag, Tag::kImg);

  // Void tags should be automatically closed
  EXPECT_EQ(created_nodes_[1]->in(), 2);
  EXPECT_EQ(created_nodes_[1]->out(), 3);
  EXPECT_EQ(created_nodes_[2]->in(), 4);
  EXPECT_EQ(created_nodes_[2]->out(), 5);

  EXPECT_TRUE(builder_.Validate());
}

// Test: Text content
// HTML Structure: <p>Hello, World!</p>
TEST_F(DOMBuilderTest, TextContent) {
  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // Text: "Hello, World!"
  HtmlTextToken text_token;
  text_token.text_content = "Hello, World!";
  EXPECT_TRUE(builder_.FeedTextToken(std::move(text_token)));

  // </p>
  HtmlCloseToken p_close;
  p_close.tag = Tag::kP;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(p_close)));

  EXPECT_EQ(created_nodes_.size(), 1);
  EXPECT_EQ(created_nodes_[0]->text_content(), "Hello, World!");
  EXPECT_TRUE(builder_.Validate());
}

// Test: Complex HTML structure
// HTML Structure:
//   <html>
//     <body>
//       <div>
//         <h1>Title</h1>
//         <p>Content</p>
//       </div>
//     </body>
//   </html>
TEST_F(DOMBuilderTest, ComplexStructure) {
  // <html>
  HtmlToken html_open;
  html_open.tag = Tag::kHtml;
  html_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(html_open)));

  // <body>
  HtmlToken body_open;
  body_open.tag = Tag::kBody;
  body_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(body_open)));

  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <h1>
  HtmlToken h1_open;
  h1_open.tag = Tag::kH1;
  h1_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(h1_open)));

  // Text: "Title"
  HtmlTextToken title_text;
  title_text.text_content = "Title";
  EXPECT_TRUE(builder_.FeedTextToken(std::move(title_text)));

  // </h1>
  HtmlCloseToken h1_close;
  h1_close.tag = Tag::kH1;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(h1_close)));

  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // Text: "Content"
  HtmlTextToken content_text;
  content_text.text_content = "Content";
  EXPECT_TRUE(builder_.FeedTextToken(std::move(content_text)));

  // </p>
  HtmlCloseToken p_close;
  p_close.tag = Tag::kP;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(p_close)));

  // </div>
  HtmlCloseToken div_close;
  div_close.tag = Tag::kDiv;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(div_close)));

  // </body>
  HtmlCloseToken body_close;
  body_close.tag = Tag::kBody;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(body_close)));

  // </html>
  HtmlCloseToken html_close;
  html_close.tag = Tag::kHtml;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(html_close)));

  // Verify 5 nodes were created (html, body, div, h1, p)
  EXPECT_EQ(created_nodes_.size(), 5);
  EXPECT_EQ(created_nodes_[0]->html_token().tag, Tag::kHtml);
  EXPECT_EQ(created_nodes_[1]->html_token().tag, Tag::kBody);
  EXPECT_EQ(created_nodes_[2]->html_token().tag, Tag::kDiv);
  EXPECT_EQ(created_nodes_[3]->html_token().tag, Tag::kH1);
  EXPECT_EQ(created_nodes_[4]->html_token().tag, Tag::kP);

  EXPECT_EQ(created_nodes_[3]->text_content(), "Title");
  EXPECT_EQ(created_nodes_[4]->text_content(), "Content");

  EXPECT_TRUE(builder_.Validate());
}

// Test: Mismatched closing tag
// HTML Structure (invalid):
//   <div>
//     <p>
//   </div>  <!-- Error: closing div before p -->
TEST_F(DOMBuilderTest, MismatchedClosingTag) {
  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // </div> (wrong order - p is still open)
  HtmlCloseToken div_close;
  div_close.tag = Tag::kDiv;
  EXPECT_FALSE(builder_.FeedCloseToken(std::move(div_close)));

  // Stack should still not be empty
  EXPECT_FALSE(builder_.Validate());
}

// Test: Text token without open tag
// HTML Structure (invalid): "Some text" (no parent tag)
TEST_F(DOMBuilderTest, TextTokenWithoutOpenTag) {
  // Try to add text token without any open tag
  HtmlTextToken text_token;
  text_token.text_content = "Some text";
  EXPECT_FALSE(builder_.FeedTextToken(std::move(text_token)));
  EXPECT_TRUE(builder_.Validate());
}

// Test: Incomplete DOM structure
// HTML Structure (incomplete):
//   <div>
//     <p>
//   <!-- Missing closing tags -->
TEST_F(DOMBuilderTest, IncompleteDOM) {
  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // Don't close div and p
  EXPECT_FALSE(builder_.Validate());
}

// Test: Callback verification with multiple nodes
// HTML Structure:
//   <div>
//     <p>
//       <span>
//   <!-- Intentionally left open for callback verification -->
TEST_F(DOMBuilderTest, CallbackVerification) {
  std::vector<std::uint32_t> node_ids;
  std::vector<Tag> node_tags;

  builder_.SetNodeCreationCallback(
      [&node_ids, &node_tags](std::shared_ptr<Node> node) {
        node_ids.push_back(node->id());
        node_tags.push_back(node->html_token().tag);
      });

  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // <span>
  HtmlToken span_open;
  span_open.tag = Tag::kSpan;
  span_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(span_open)));

  EXPECT_EQ(node_ids.size(), 3);
  EXPECT_EQ(node_ids[0], 0);
  EXPECT_EQ(node_ids[1], 1);
  EXPECT_EQ(node_ids[2], 2);

  EXPECT_EQ(node_tags[0], Tag::kDiv);
  EXPECT_EQ(node_tags[1], Tag::kP);
  EXPECT_EQ(node_tags[2], Tag::kSpan);
}

// Test: Euler tour ordering with sibling elements
// HTML Structure:
//   <div>
//     <p></p>
//     <span></span>
//   </div>
TEST_F(DOMBuilderTest, EulerTourOrdering) {
  // <div>
  HtmlToken div_open;
  div_open.tag = Tag::kDiv;
  div_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(div_open)));

  // <p>
  HtmlToken p_open;
  p_open.tag = Tag::kP;
  p_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(p_open)));

  // </p>
  HtmlCloseToken p_close;
  p_close.tag = Tag::kP;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(p_close)));

  // <span>
  HtmlToken span_open;
  span_open.tag = Tag::kSpan;
  span_open.is_void_tag = false;
  EXPECT_TRUE(builder_.FeedOpenToken(std::move(span_open)));

  // </span>
  HtmlCloseToken span_close;
  span_close.tag = Tag::kSpan;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(span_close)));

  // </div>
  HtmlCloseToken div_close;
  div_close.tag = Tag::kDiv;
  EXPECT_TRUE(builder_.FeedCloseToken(std::move(div_close)));

  EXPECT_EQ(created_nodes_.size(), 3);

  // Verify Euler tour timings
  // div: in=1, out=6
  EXPECT_EQ(created_nodes_[0]->in(), 1);
  EXPECT_EQ(created_nodes_[0]->out(), 6);

  // p: in=2, out=3
  EXPECT_EQ(created_nodes_[1]->in(), 2);
  EXPECT_EQ(created_nodes_[1]->out(), 3);

  // span: in=4, out=5
  EXPECT_EQ(created_nodes_[2]->in(), 4);
  EXPECT_EQ(created_nodes_[2]->out(), 5);

  EXPECT_TRUE(builder_.Validate());
}

// Test: Memory cleanup verification
// HTML Structure:
//   <div>
//     <p>
//       <span></span>
//     </p>
//     <ul>
//       <li></li>
//       <li></li>
//     </ul>
//   </div>
// Verify that all nodes are properly deallocated when references are released
TEST_F(DOMBuilderTest, MemoryCleanup) {
  std::vector<std::weak_ptr<Node>> weak_nodes;

  // Create a new scope to ensure builder and nodes go out of scope
  {
    DOMBuilder scoped_builder;
    std::vector<std::shared_ptr<Node>> scoped_nodes;

    scoped_builder.SetNodeCreationCallback(
        [&scoped_nodes, &weak_nodes](std::shared_ptr<Node> node) {
          scoped_nodes.push_back(node);
          weak_nodes.push_back(node);  // Store weak_ptr for later verification
        });

    // <div>
    HtmlToken div_open;
    div_open.tag = Tag::kDiv;
    div_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(div_open)));

    // <p>
    HtmlToken p_open;
    p_open.tag = Tag::kP;
    p_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(p_open)));

    // <span>
    HtmlToken span_open;
    span_open.tag = Tag::kSpan;
    span_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(span_open)));

    // </span>
    HtmlCloseToken span_close;
    span_close.tag = Tag::kSpan;
    EXPECT_TRUE(scoped_builder.FeedCloseToken(std::move(span_close)));

    // </p>
    HtmlCloseToken p_close;
    p_close.tag = Tag::kP;
    EXPECT_TRUE(scoped_builder.FeedCloseToken(std::move(p_close)));

    // <ul>
    HtmlToken ul_open;
    ul_open.tag = Tag::kUl;
    ul_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(ul_open)));

    // <li>
    HtmlToken li1_open;
    li1_open.tag = Tag::kLi;
    li1_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(li1_open)));

    // </li>
    HtmlCloseToken li1_close;
    li1_close.tag = Tag::kLi;
    EXPECT_TRUE(scoped_builder.FeedCloseToken(std::move(li1_close)));

    // <li>
    HtmlToken li2_open;
    li2_open.tag = Tag::kLi;
    li2_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(li2_open)));

    // </li>
    HtmlCloseToken li2_close;
    li2_close.tag = Tag::kLi;
    EXPECT_TRUE(scoped_builder.FeedCloseToken(std::move(li2_close)));

    // </ul>
    HtmlCloseToken ul_close;
    ul_close.tag = Tag::kUl;
    EXPECT_TRUE(scoped_builder.FeedCloseToken(std::move(ul_close)));

    // </div>
    HtmlCloseToken div_close;
    div_close.tag = Tag::kDiv;
    EXPECT_TRUE(scoped_builder.FeedCloseToken(std::move(div_close)));

    EXPECT_TRUE(scoped_builder.Validate());

    // Verify 6 nodes were created (div, p, span, ul, li, li)
    EXPECT_EQ(scoped_nodes.size(), 6);
    EXPECT_EQ(weak_nodes.size(), 6);

    // All weak_ptrs should still be valid since nodes are still in scope
    for (const auto& weak_node : weak_nodes) {
      EXPECT_FALSE(weak_node.expired());
    }
  }
  // End of scope - scoped_builder and scoped_nodes are destroyed

  // After scope ends, all nodes should be deallocated
  // All weak_ptrs should be expired now
  for (const auto& weak_node : weak_nodes) {
    EXPECT_TRUE(weak_node.expired())
        << "Node was not properly deallocated - memory leak detected!";
  }
}

// Test: Memory cleanup with partial structure
// HTML Structure (intentionally incomplete):
//   <div>
//     <p>
//   <!-- No closing tags - tests cleanup of incomplete DOM -->
TEST_F(DOMBuilderTest, MemoryCleanupIncompleteDOM) {
  std::vector<std::weak_ptr<Node>> weak_nodes;

  {
    DOMBuilder scoped_builder;
    std::vector<std::shared_ptr<Node>> scoped_nodes;

    scoped_builder.SetNodeCreationCallback(
        [&scoped_nodes, &weak_nodes](std::shared_ptr<Node> node) {
          scoped_nodes.push_back(node);
          weak_nodes.push_back(node);
        });

    // <div>
    HtmlToken div_open;
    div_open.tag = Tag::kDiv;
    div_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(div_open)));

    // <p>
    HtmlToken p_open;
    p_open.tag = Tag::kP;
    p_open.is_void_tag = false;
    EXPECT_TRUE(scoped_builder.FeedOpenToken(std::move(p_open)));

    // Intentionally don't close tags
    EXPECT_FALSE(scoped_builder.Validate());
    EXPECT_EQ(scoped_nodes.size(), 2);

    // Weak pointers should still be valid
    for (const auto& weak_node : weak_nodes) {
      EXPECT_FALSE(weak_node.expired());
    }
  }

  // Even with incomplete DOM, all nodes should be deallocated
  for (const auto& weak_node : weak_nodes) {
    EXPECT_TRUE(weak_node.expired())
        << "Node from incomplete DOM was not deallocated - memory leak!";
  }
}

}  // namespace arboris
