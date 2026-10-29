/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_HTML_TOKENS_HPP_
#define SRC_UTILS_HTML_TOKENS_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "utils/tag.hpp"
#include "utils/tokens.hpp"

namespace arboris {

// Query inputs still own their strings: they come from the caller, not from the document.
using AttributeMap = std::unordered_map<std::string, std::unordered_set<std::string>>;
using ClassSet = std::unordered_set<std::string>;

// One attribute of an open tag, pointing into the document buffer DOMStore owns.
//
// An attribute written without a value has an empty value, as the HTML standard says.
// has_value() tells the two spellings apart for anything that has to write the document
// back out: `disabled` leaves value with no data, `disabled=""` points at the empty run.
struct Attribute {
  std::string_view name;
  std::string_view value;

  [[nodiscard]] bool has_value() const noexcept {
    return value.data() != nullptr;
  }
};

// One run of text directly inside a tag, pointing into the string pool. Keeping the
// source position lets a reader put the run back in order against the tag's children.
struct TextRun {
  std::string_view text;
  std::uint32_t begin_pos = 0;
};

struct BaseHtmlToken : public BaseToken {};

struct HtmlToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;
  bool is_void_tag = false;

  // The tag ends in "/>". An HTML element ignores it, so only an element inside <svg> or
  // <math>, where it means what it does in XML, is closed on it.
  bool is_self_closing = false;

  // Into the document buffer.
  std::string_view id;

  // Ranges into DOMStore's arenas, not spans: the arenas keep growing while the document
  // is parsed, and a reallocation would leave every span dangling.
  std::uint32_t class_begin = 0;
  std::uint32_t class_count = 0;
  std::uint32_t attr_begin = 0;
  std::uint32_t attr_count = 0;
};

struct HtmlTextToken : public BaseHtmlToken {
  std::string_view text_content;
};

struct HtmlCloseToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;
};

// `<!-- data -->`, or a bogus comment: `<?data>` keeps its '?', `<!data>` drops its '!'.
// data points into the document buffer.
struct HtmlCommentToken : public BaseHtmlToken {
  std::string_view data;
};

// `<!DOCTYPE name PUBLIC "public id" "system id">`, every part pointing into the document
// buffer. The name is kept as written; HTML compares it ignoring ASCII case.
//
// force_quirks is set on one that is not well-formed. An id written as "" is an empty
// view, one not written at all is nullopt.
struct HtmlDoctypeToken : public BaseHtmlToken {
  std::string_view name;
  std::optional<std::string_view> public_id;
  std::optional<std::string_view> system_id;
  bool force_quirks = false;
};

}  // namespace arboris

#endif  // SRC_UTILS_HTML_TOKENS_HPP_
