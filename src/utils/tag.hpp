/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_TAG_HPP_
#define SRC_UTILS_TAG_HPP_

#include <array>
#include <cstdint>
#include <initializer_list>
#include <string_view>

namespace arboris {

enum class Tag : std::uint8_t {
  kUnknown,
  kA,
  kAbbr,
  kAddress,
  kArea,
  kArticle,
  kAside,
  kAudio,
  kB,
  kBase,
  kBdi,
  kBdo,
  kBlockquote,
  kBody,
  kBr,
  kButton,
  kCanvas,
  kCaption,
  kCite,
  kCode,
  kCol,
  kColgroup,
  kData,
  kDatalist,
  kDd,
  kDel,
  kDetails,
  kDfn,
  kDialog,
  kDiv,
  kDl,
  kDt,
  kEm,
  kEmbed,
  kFieldset,
  kFigcaption,
  kFigure,
  kFooter,
  kForm,
  kH1,
  kH2,
  kH3,
  kH4,
  kH5,
  kH6,
  kHead,
  kHeader,
  kHgroup,
  kHr,
  kHtml,
  kI,
  kIframe,
  kImg,
  kInput,
  kIns,
  kKbd,
  kLabel,
  kLegend,
  kLi,
  kLink,
  kMain,
  kMap,
  kMark,
  kMenu,
  kMeta,
  kMeter,
  kNav,
  kNoscript,
  kObject,
  kOl,
  kOptgroup,
  kOption,
  kOutput,
  kP,
  kPicture,
  kPre,
  kProgress,
  kQ,
  kRp,
  kRt,
  kRuby,
  kS,
  kSamp,
  kScript,
  kSearch,
  kSection,
  kSelect,
  kSmall,
  kSource,
  kSpan,
  kStrong,
  kStyle,
  kSub,
  kSummary,
  kSup,
  kTable,
  kTbody,
  kTd,
  kTemplate,
  kTextarea,
  kTfoot,
  kTh,
  kThead,
  kTime,
  kTitle,
  kTr,
  kTrack,
  kU,
  kUl,
  kVar,
  kVideo,
  kWbr,
};

struct TagSet {
  std::uint64_t bits[static_cast<std::uint8_t>(Tag::kWbr) / 64 + 1] = {0};
};

Tag FromString(std::string_view tag_name);

constexpr TagSet CreateTagSet(std::initializer_list<Tag> tags) {
  TagSet tag_set{};
  for (const auto tag : tags) {
    std::uint8_t tag_index = static_cast<std::uint8_t>(tag);
    tag_set.bits[tag_index >> 6] |= (1ULL << (tag_index & 63));
  }
  return tag_set;
}

constexpr bool ContainsTag(TagSet tag_set, Tag tag) {
  std::size_t tag_index = static_cast<std::size_t>(tag);
  return (tag_set.bits[tag_index >> 6] & (1ULL << (tag_index & 63))) != 0;
}

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

}  // namespace arboris

#endif  // SRC_UTILS_TAG_HPP_
