/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/tag_rules.hpp"

namespace arboris {

namespace {

constexpr TagSet kEndsP = CreateTagSet({
    Tag::kAddress, Tag::kArticle, Tag::kAside, Tag::kBlockquote, Tag::kDd, Tag::kDetails,
    Tag::kDialog, Tag::kDiv, Tag::kDl, Tag::kDt, Tag::kFieldset, Tag::kFigcaption, Tag::kFigure,
    Tag::kFooter, Tag::kForm, Tag::kH1, Tag::kH2, Tag::kH3, Tag::kH4, Tag::kH5, Tag::kH6,
    Tag::kHeader, Tag::kHgroup, Tag::kHr, Tag::kLi, Tag::kMain, Tag::kMenu, Tag::kNav, Tag::kOl,
    Tag::kP, Tag::kPre, Tag::kSearch, Tag::kSection, Tag::kSummary, Tag::kTable, Tag::kUl,
});

constexpr TagSet kHeadContent = CreateTagSet({
    Tag::kBase, Tag::kLink, Tag::kMeta, Tag::kNoscript, Tag::kScript, Tag::kStyle, Tag::kTemplate,
    Tag::kTitle,
});

constexpr TagSet kTableSections = CreateTagSet({Tag::kThead, Tag::kTbody, Tag::kTfoot});
constexpr TagSet kTableRows = CreateTagSet({Tag::kTr, Tag::kThead, Tag::kTbody, Tag::kTfoot});

}  // anonymous namespace

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

}  // namespace arboris
