/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <unordered_map>

#include "utils/tag.hpp"

namespace arboris {

Tag FromString(std::string_view tag_name) {
  static const std::unordered_map<std::string_view, Tag> kTagMap = {
      {"a", Tag::kA},
      {"abbr", Tag::kAbbr},
      {"address", Tag::kAddress},
      {"area", Tag::kArea},
      {"article", Tag::kArticle},
      {"aside", Tag::kAside},
      {"audio", Tag::kAudio},
      {"b", Tag::kB},
      {"base", Tag::kBase},
      {"bdi", Tag::kBdi},
      {"bdo", Tag::kBdo},
      {"blockquote", Tag::kBlockquote},
      {"body", Tag::kBody},
      {"br", Tag::kBr},
      {"button", Tag::kButton},
      {"canvas", Tag::kCanvas},
      {"caption", Tag::kCaption},
      {"cite", Tag::kCite},
      {"code", Tag::kCode},
      {"col", Tag::kCol},
      {"colgroup", Tag::kColgroup},
      {"data", Tag::kData},
      {"datalist", Tag::kDatalist},
      {"dd", Tag::kDd},
      {"del", Tag::kDel},
      {"details", Tag::kDetails},
      {"dfn", Tag::kDfn},
      {"dialog", Tag::kDialog},
      {"div", Tag::kDiv},
      {"dl", Tag::kDl},
      {"dt", Tag::kDt},
      {"em", Tag::kEm},
      {"embed", Tag::kEmbed},
      {"fieldset", Tag::kFieldset},
      {"figcaption", Tag::kFigcaption},
      {"figure", Tag::kFigure},
      {"footer", Tag::kFooter},
      {"form", Tag::kForm},
      {"h1", Tag::kH1},
      {"h2", Tag::kH2},
      {"h3", Tag::kH3},
      {"h4", Tag::kH4},
      {"h5", Tag::kH5},
      {"h6", Tag::kH6},
      {"head", Tag::kHead},
      {"header", Tag::kHeader},
      {"hgroup", Tag::kHgroup},
      {"hr", Tag::kHr},
      {"html", Tag::kHtml},
      {"i", Tag::kI},
      {"iframe", Tag::kIframe},
      {"img", Tag::kImg},
      {"input", Tag::kInput},
      {"ins", Tag::kIns},
      {"kbd", Tag::kKbd},
      {"label", Tag::kLabel},
      {"legend", Tag::kLegend},
      {"li", Tag::kLi},
      {"link", Tag::kLink},
      {"main", Tag::kMain},
      {"map", Tag::kMap},
      {"mark", Tag::kMark},
      {"menu", Tag::kMenu},
      {"meta", Tag::kMeta},
      {"meter", Tag::kMeter},
      {"nav", Tag::kNav},
      {"noscript", Tag::kNoscript},
      {"object", Tag::kObject},
      {"ol", Tag::kOl},
      {"optgroup", Tag::kOptgroup},
      {"option", Tag::kOption},
      {"output", Tag::kOutput},
      {"p", Tag::kP},
      {"picture", Tag::kPicture},
      {"pre", Tag::kPre},
      {"progress", Tag::kProgress},
      {"q", Tag::kQ},
      {"rp", Tag::kRp},
      {"rt", Tag::kRt},
      {"ruby", Tag::kRuby},
      {"s", Tag::kS},
      {"samp", Tag::kSamp},
      {"script", Tag::kScript},
      {"search", Tag::kSearch},
      {"section", Tag::kSection},
      {"select", Tag::kSelect},
      {"small", Tag::kSmall},
      {"source", Tag::kSource},
      {"span", Tag::kSpan},
      {"strong", Tag::kStrong},
      {"style", Tag::kStyle},
      {"sub", Tag::kSub},
      {"summary", Tag::kSummary},
      {"sup", Tag::kSup},
      {"table", Tag::kTable},
      {"tbody", Tag::kTbody},
      {"td", Tag::kTd},
      {"template", Tag::kTemplate},
      {"textarea", Tag::kTextarea},
      {"tfoot", Tag::kTfoot},
      {"th", Tag::kTh},
      {"thead", Tag::kThead},
      {"time", Tag::kTime},
      {"title", Tag::kTitle},
      {"tr", Tag::kTr},
      {"track", Tag::kTrack},
      {"u", Tag::kU},
      {"ul", Tag::kUl},
      {"var", Tag::kVar},
      {"video", Tag::kVideo},
      {"wbr", Tag::kWbr},
  };

  auto it = kTagMap.find(tag_name);
  if (it == kTagMap.end()) {
    return Tag::kUnknown;
  }
  return it->second;
}

bool IsVoidTag(Tag tag) {
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

bool ContainsTag(TagSet tag_set, Tag tag) {
  std::size_t tag_index = static_cast<std::size_t>(tag);
  return tag_set.bits[tag_index >> 6] & (1ULL << (tag_index & 63));
}

constexpr TagSet CreateTagSet(std::initializer_list<Tag> tags) {
  TagSet tag_set{};
  for (const auto tag : tags) {
    std::uint8_t tag_index = static_cast<std::uint8_t>(tag);
    tag_set.bits[tag_index >> 6] |= (1ULL << (tag_index & 63));
  }
  return tag_set;
}

}  // namespace arboris
