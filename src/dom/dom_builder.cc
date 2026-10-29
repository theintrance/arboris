/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "dom/dom_builder.hpp"
#include "dom/tag_node.hpp"
#include "string/string.hpp"

namespace arboris {

namespace {

// Elements whose end tag the HTML standard lets an author leave out. A tag that implies one
// of these ends closes only them on its way down the open elements, so an element the
// author had to close is never closed for them.
constexpr TagSet kOptionalEndTags = CreateTagSet({
    Tag::kHtml, Tag::kHead, Tag::kBody, Tag::kP, Tag::kLi, Tag::kDt, Tag::kDd, Tag::kRt, Tag::kRp,
    Tag::kOptgroup, Tag::kOption, Tag::kColgroup, Tag::kCaption, Tag::kThead, Tag::kTbody,
    Tag::kTfoot, Tag::kTr, Tag::kTd, Tag::kTh,
});

// The start tags that end an open <p>: blocks a paragraph cannot hold.
constexpr TagSet kEndsP = CreateTagSet({
    Tag::kAddress, Tag::kArticle, Tag::kAside, Tag::kBlockquote, Tag::kDd, Tag::kDetails,
    Tag::kDialog, Tag::kDiv, Tag::kDl, Tag::kDt, Tag::kFieldset, Tag::kFigcaption, Tag::kFigure,
    Tag::kFooter, Tag::kForm, Tag::kH1, Tag::kH2, Tag::kH3, Tag::kH4, Tag::kH5, Tag::kH6,
    Tag::kHeader, Tag::kHgroup, Tag::kHr, Tag::kLi, Tag::kMain, Tag::kMenu, Tag::kNav, Tag::kOl,
    Tag::kP, Tag::kPre, Tag::kSearch, Tag::kSection, Tag::kSummary, Tag::kTable, Tag::kUl,
});

// What <head> may hold. Any other start tag ends it, <body> among them.
constexpr TagSet kHeadContent = CreateTagSet({
    Tag::kBase, Tag::kLink, Tag::kMeta, Tag::kNoscript, Tag::kScript, Tag::kStyle, Tag::kTemplate,
    Tag::kTitle,
});

constexpr TagSet kTableSections = CreateTagSet({Tag::kThead, Tag::kTbody, Tag::kTfoot});
constexpr TagSet kTableRows = CreateTagSet({Tag::kTr, Tag::kThead, Tag::kTbody, Tag::kTfoot});

// Whether a start tag ends an open element whose end tag was left out: a <li> the <li>
// before it, a <div> the <p> it cannot sit in, a <tr> the cell and the row before it.
// These are the omissions the HTML standard allows, read the way its tree builder reads them.
bool StartTagEnds(Tag open_tag, Tag start_tag) {
  switch (open_tag) {
    case Tag::kP:
      return ContainsTag(kEndsP, start_tag);
    case Tag::kLi:
      return start_tag == Tag::kLi;
    case Tag::kDt:
    case Tag::kDd:
      return start_tag == Tag::kDt || start_tag == Tag::kDd;
    case Tag::kRt:
    case Tag::kRp:
      return start_tag == Tag::kRt || start_tag == Tag::kRp;
    case Tag::kOption:
      return start_tag == Tag::kOption || start_tag == Tag::kOptgroup;
    case Tag::kOptgroup:
      return start_tag == Tag::kOptgroup;
    case Tag::kTd:
    case Tag::kTh:
      return start_tag == Tag::kTd || start_tag == Tag::kTh || ContainsTag(kTableRows, start_tag);
    case Tag::kTr:
      return ContainsTag(kTableRows, start_tag);
    case Tag::kThead:
    case Tag::kTbody:
    case Tag::kTfoot:
      return ContainsTag(kTableSections, start_tag);
    case Tag::kCaption:
      return StartTagEnds(Tag::kTd, start_tag) || start_tag == Tag::kCaption || start_tag == Tag::kColgroup ||
             start_tag == Tag::kCol;
    case Tag::kColgroup:
      return start_tag != Tag::kCol;
    case Tag::kHead:
      return !ContainsTag(kHeadContent, start_tag);
    default:
      return false;
  }
}

bool IsForeignRoot(Tag tag) {
  return tag == Tag::kSvg || tag == Tag::kMath;
}

}  // anonymous namespace

bool DOMBuilder::Validate() const {
  return node_stack_.empty();
}

bool DOMBuilder::FeedOpenToken(HtmlToken&& token, std::span<const Attribute> attributes, const char* text_begin) {
  closeElementsEndedBy(token.tag, text_begin);
  storeAttributes(&token, attributes);

  // In <svg> and <math>, "/>" closes an element as it does in XML. In HTML only a void
  // element closes on its own, and the '/' of `<div/>` means nothing.
  const Tag tag = token.tag;
  const bool closes_at_once =
      token.is_void_tag || (token.is_self_closing && (foreign_depth_ > 0 || IsForeignRoot(tag)));

  const NodeKey parent_key = openNode().key();
  const NodeKey key = next_node_key_++;

  auto& node = dfs_node_list_.emplace_back(key, parent_key, std::move(token));
  node.set_text_content({text_begin, 0});  // NOLINT(bugprone-string-constructor)

  node_stack_.push_back(key);
  pending_marks_.push_back(static_cast<std::uint32_t>(pending_runs_.size()));
  if (IsForeignRoot(tag)) {
    ++foreign_depth_;
  }

  if (closes_at_once) {
    return closeTopNode(text_begin);
  }
  return true;
}

// Closes the open elements a start tag implies the end of, as a <li> does a <li> whose
// `</li>` was left out. The search runs down from the top through elements whose end tag
// may be left out, and starts over from the new top after each close: a <tr> ends the cell
// and then the row it was in.
void DOMBuilder::closeElementsEndedBy(Tag start_tag, const char* text_end) {
  for (std::size_t depth = node_stack_.size(); depth > 0;) {
    // Checked first because it settles most start tags at once: only an element whose end
    // tag may be left out is ever ended by one.
    const Tag open_tag = tagAt(depth - 1);
    if (!ContainsTag(kOptionalEndTags, open_tag)) {
      return;
    }

    if (StartTagEnds(open_tag, start_tag)) {
      closeDownTo(depth - 1, text_end);
      depth = node_stack_.size();
    } else {
      --depth;
    }
  }
}

// Copies the attributes out of the parser's array, which the next tag reuses, and reads the
// two the DOM gives a meaning: the id, and the class names. Both stay among the attributes
// too, so a query for [id] or [class] still finds them.
void DOMBuilder::storeAttributes(HtmlToken* token, std::span<const Attribute> attributes) {
  token->attr_begin = static_cast<std::uint32_t>(attr_arena_.size());
  token->attr_count = static_cast<std::uint32_t>(attributes.size());
  attr_arena_.insert(attr_arena_.end(), attributes.begin(), attributes.end());

  token->class_begin = static_cast<std::uint32_t>(class_arena_.size());
  for (const auto& attribute : attributes) {
    if (attribute.name == "id") {
      token->id = attribute.value;
    } else if (attribute.name == "class") {
      appendClasses(attribute.value, token->class_begin);
    }
  }
  token->class_count = static_cast<std::uint32_t>(class_arena_.size()) - token->class_begin;
}

// Splits a class attribute on whitespace, as classList does, and keeps each name once: a
// repeated name would list the node twice in the class index, and FindAll would return it
// twice.
void DOMBuilder::appendClasses(std::string_view class_list, std::uint32_t run_begin) {
  std::size_t pos = SkipHtmlWhitespace(class_list, 0);
  while (pos < class_list.length()) {
    const std::size_t end = std::min(FindNextAnyChar(class_list, pos, kHtmlWhitespace), class_list.length());
    const std::string_view class_name = class_list.substr(pos, end - pos);

    const auto run = std::span(class_arena_).subspan(run_begin);
    if (std::ranges::find(run, class_name) == run.end()) {
      class_arena_.push_back(class_name);
    }
    pos = SkipHtmlWhitespace(class_list, end);
  }
}

bool DOMBuilder::FeedTextToken(HtmlTextToken&& token) {
  pending_runs_.push_back(TextRun{token.text_content, token.begin_pos});
  return true;
}

// Moves the runs a node collected while it was open into the arena, side by side.
void DOMBuilder::flushPendingRuns(TagNode* node) {
  const auto mark = pending_marks_.empty() ? 0U : pending_marks_.back();
  if (!pending_marks_.empty()) {
    pending_marks_.pop_back();
  }

  const auto begin = static_cast<std::uint32_t>(text_arena_.size());
  text_arena_.insert(text_arena_.end(), pending_runs_.begin() + mark, pending_runs_.end());
  pending_runs_.resize(mark);

  node->set_text_runs(begin, static_cast<std::uint32_t>(text_arena_.size()) - begin);
}

void DOMBuilder::Finish(const char* text_end) {
  // Elements whose end tag was left out, `</body>` and `</html>` among them, end with the
  // document. So do those a parse that stopped early left open: their subtree size would
  // stay 0, and a walk that steps by subtree size would never move on.
  closeDownTo(0, text_end);

  flushPendingRuns(&root());
  root().set_sub_tree_size(static_cast<std::uint32_t>(dfs_node_list_.size()));
}

// Closes the element an end tag names, and on the way the open elements whose end tags
// were left out: `</ul>` closes the last <li>, `</div>` a <p> inside it.
//
// An end tag for an element that is not open at all is dropped, as the HTML tree builder
// drops it: the `</p>` of `<p><div></div></p>`, whose <p> the <div> already ended, or a
// void element's, as in `<br></br>`. One that would have to close an element the author
// left open, as `</div>` does in `<div><span></div>`, is misnested and fails the parse.
bool DOMBuilder::FeedCloseToken(HtmlCloseToken&& token, const char* text_end) {
  for (std::size_t depth = node_stack_.size(); depth > 0; --depth) {
    const Tag open_tag = tagAt(depth - 1);
    if (open_tag == token.tag) {
      closeDownTo(depth - 1, text_end);
      return true;
    }
    if (!ContainsTag(kOptionalEndTags, open_tag)) {
      const auto below = std::span(node_stack_).first(depth - 1);
      return std::ranges::none_of(below, [&](NodeKey key) { return dfs_node_list_[key].tag() == token.tag; });
    }
  }
  return true;
}

// Closes open elements until depth of them are left.
void DOMBuilder::closeDownTo(std::size_t depth, const char* text_end) {
  while (node_stack_.size() > depth) {
    closeTopNode(text_end);
  }
}

// Closes the innermost open element where its text ends, at text_end.
bool DOMBuilder::closeTopNode(const char* text_end) {
  ARBORIS_ASSERT(!node_stack_.empty(), "Node stack is empty");

  if (node_stack_.empty()) {
    return false;
  }

  TagNode& top_node = dfs_node_list_[node_stack_.back()];
  node_stack_.pop_back();
  if (IsForeignRoot(top_node.tag())) {
    --foreign_depth_;
  }

  const auto* text_begin = top_node.text_content().data();
  top_node.set_text_content({text_begin, static_cast<std::size_t>(text_end - text_begin)});
  flushPendingRuns(&top_node);

  // Keys are handed out in DFS order, so everything created since this node opened is
  // inside it.
  top_node.set_sub_tree_size(next_node_key_ - top_node.key());

  return true;
}

}  // namespace arboris
