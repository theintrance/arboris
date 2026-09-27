/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_HTML_TOKENS_HPP_
#define SRC_UTILS_HTML_TOKENS_HPP_

#include <cstdint>
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

struct BaseHtmlToken : public BaseToken {};

struct HtmlToken : public BaseHtmlToken {
  Tag tag = Tag::kUnknown;
  bool is_void_tag = false;

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

}  // namespace arboris

#endif  // SRC_UTILS_HTML_TOKENS_HPP_
