/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_TAG_RULES_HPP_
#define SRC_UTILS_TAG_RULES_HPP_

#include "utils/tag.hpp"

namespace arboris {

inline bool IsVoidTag(Tag tag) {
  static constexpr auto kVoidTags = CreateTagSet({
    Tag::kArea,
    Tag::kBase,
    Tag::kBr,
    Tag::kCol,
    Tag::kEmbed,
    Tag::kHr,
    Tag::kImg,
    Tag::kInput,
    Tag::kLink,
    Tag::kMeta,
    Tag::kSource,
    Tag::kTrack,
    Tag::kWbr,
  });

  return ContainsTag(kVoidTags, tag);
}

// Raw text (<script>, <style>) and RCDATA (<textarea>, <title>).
inline bool IsRawTextTag(Tag tag) {
  static constexpr auto kRawTextTags = CreateTagSet({Tag::kScript, Tag::kStyle, Tag::kTextarea, Tag::kTitle});
  return ContainsTag(kRawTextTags, tag);
}

// The raw text elements whose contents still decode character references.
inline bool IsRcdataTag(Tag tag) {
  return tag == Tag::kTextarea || tag == Tag::kTitle;
}

inline bool IsForeignRoot(Tag tag) {
  return tag == Tag::kSvg || tag == Tag::kMath;
}

inline bool HasOptionalEndTag(Tag tag) {
  static constexpr auto kOptionalEndTags = CreateTagSet({
    Tag::kHtml, Tag::kHead, Tag::kBody, Tag::kP, Tag::kLi, Tag::kDt, Tag::kDd, Tag::kRt, Tag::kRp,
    Tag::kOptgroup, Tag::kOption, Tag::kColgroup, Tag::kCaption, Tag::kThead, Tag::kTbody,
    Tag::kTfoot, Tag::kTr, Tag::kTd, Tag::kTh,
  });

  return ContainsTag(kOptionalEndTags, tag);
}

// Whether start_tag implies the end of an open open_tag whose end tag was left out.
bool StartTagEnds(Tag open_tag, Tag start_tag);

}  // namespace arboris

#endif  // SRC_UTILS_TAG_RULES_HPP_
