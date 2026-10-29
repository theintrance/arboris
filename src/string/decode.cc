/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "string/decode.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "string/string.hpp"

namespace arboris {

namespace {

struct NamedCharacterReference {
  std::string_view name;
  std::string_view text;
};

#include "string/named_references.inc"  // NOLINT(build/include)

static_assert(std::ranges::is_sorted(kNamedReferences, {}, &NamedCharacterReference::name));
static_assert(std::ranges::is_sorted(kLegacyReferences, {}, &NamedCharacterReference::name));

constexpr std::size_t kLongestName = 32;
constexpr std::size_t kLongestLegacyName = 6;

// The references pages use most, settled before the table is searched.
constexpr NamedCharacterReference kCommonReferences[] = {
    {"amp;", "&"}, {"lt;", "<"}, {"gt;", ">"}, {"quot;", "\""}, {"nbsp;", "\xc2\xa0"},
};

constexpr CharSet kTextSpecials{"\r&"};
constexpr CharSet kRawTextSpecials{"\r"};

constexpr std::string_view kReplacementCharacter = "\xef\xbf\xbd";

// What the HTML tokenizer makes of a numeric reference to a C1 control: the windows-1252
// character at that byte, a legacy that pages still rely on.
constexpr std::array<char32_t, 32> kC1Replacements = {
    0x20AC, 0x81,   0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
    0x2039, 0x0152, 0x8D,   0x017D, 0x8F,   0x90,   0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x9D,   0x017E, 0x0178,
};

bool IsAsciiAlphanumeric(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int HexDigit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

// Where decoded bytes go. Never checks for room: the caller sized the buffer by
// DecodedSizeBound.
class Writer {
 public:
  explicit Writer(char* out) : begin_(out), cursor_(out) {}

  void Put(char c) {
    *cursor_++ = c;
  }

  void Put(std::string_view str) {
    cursor_ = std::ranges::copy(str, cursor_).out;
  }

  void PutUtf8(char32_t code_point) {
    if (code_point < 0x80) {
      Put(static_cast<char>(code_point));
    } else if (code_point < 0x800) {
      Put(static_cast<char>(0xC0 | (code_point >> 6)));
      Put(static_cast<char>(0x80 | (code_point & 0x3F)));
    } else if (code_point < 0x10000) {
      Put(static_cast<char>(0xE0 | (code_point >> 12)));
      Put(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
      Put(static_cast<char>(0x80 | (code_point & 0x3F)));
    } else {
      Put(static_cast<char>(0xF0 | (code_point >> 18)));
      Put(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
      Put(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
      Put(static_cast<char>(0x80 | (code_point & 0x3F)));
    }
  }

  [[nodiscard]] std::size_t written() const {
    return static_cast<std::size_t>(cursor_ - begin_);
  }

 private:
  char* begin_;
  char* cursor_;
};

const NamedCharacterReference* Lookup(std::span<const NamedCharacterReference> table, std::string_view name) {
  const auto it = std::ranges::lower_bound(table, name, {}, &NamedCharacterReference::name);
  return it != table.end() && it->name == name ? &*it : nullptr;
}

// `rest` starts just past "&#". Returns how much of it the reference took, or 0 when
// there are no digits, in which case nothing is appended.
std::size_t DecodeNumeric(std::string_view rest, Writer* out) {
  std::size_t pos = 0;
  const bool hex = !rest.empty() && (rest[0] == 'x' || rest[0] == 'X');
  if (hex) {
    ++pos;
  }

  const std::size_t digits_begin = pos;
  char32_t code_point = 0;
  for (; pos < rest.size(); ++pos) {
    const int digit = hex ? HexDigit(rest[pos]) : (rest[pos] >= '0' && rest[pos] <= '9' ? rest[pos] - '0' : -1);
    if (digit < 0) {
      break;
    }
    code_point = std::min<char32_t>(code_point * (hex ? 16 : 10) + static_cast<char32_t>(digit), 0x110000);
  }
  if (pos == digits_begin) {
    return 0;
  }
  if (pos < rest.size() && rest[pos] == ';') {
    ++pos;
  }

  if (code_point >= 0x80 && code_point <= 0x9F) {
    code_point = kC1Replacements[code_point - 0x80];
  }
  if (code_point == 0 || code_point > 0x10FFFF || (code_point >= 0xD800 && code_point <= 0xDFFF)) {
    out->Put(kReplacementCharacter);
  } else {
    out->PutUtf8(code_point);
  }
  return pos;
}

// `rest` starts just past "&". Returns how much of it the reference took, or 0 when it is
// not a reference, in which case nothing is appended.
std::size_t DecodeNamed(std::string_view rest, TextMode mode, Writer* out) {
  std::size_t run = 0;
  while (run < rest.size() && run < kLongestName && IsAsciiAlphanumeric(rest[run])) {
    ++run;
  }
  if (run == 0) {
    return 0;
  }

  if (run < rest.size() && rest[run] == ';') {
    const std::string_view name = rest.substr(0, run + 1);
    for (const auto& common : kCommonReferences) {
      if (common.name == name) {
        out->Put(common.text);
        return run + 1;
      }
    }
    if (const auto* found = Lookup(kNamedReferences, name)) {
      out->Put(found->text);
      return run + 1;
    }
  }

  // The longest legacy name that is a prefix of the run; `&notit;` reads as `¬it;`.
  for (std::size_t length = std::min(run, kLongestLegacyName); length > 0; --length) {
    const auto* found = Lookup(kLegacyReferences, rest.substr(0, length));
    if (found == nullptr) {
      continue;
    }
    if (mode == TextMode::kAttributeValue && length < rest.size() &&
        (rest[length] == '=' || IsAsciiAlphanumeric(rest[length]))) {
      return 0;
    }
    out->Put(found->text);
    return length;
  }
  return 0;
}

std::size_t DecodeReference(std::string_view rest, TextMode mode, Writer* out) {
  if (rest.starts_with('#')) {
    const std::size_t taken = DecodeNumeric(rest.substr(1), out);
    return taken == 0 ? 0 : taken + 1;
  }
  return DecodeNamed(rest, mode, out);
}

}  // anonymous namespace

std::size_t DecodedSizeBound(std::size_t source_size) {
  return source_size * kReferenceGrowthNumerator / kReferenceGrowthDenominator + 1;
}

bool NeedsDecoding(std::string_view in, TextMode mode) {
  return in.find('\r') != std::string_view::npos ||
         (mode != TextMode::kRawText && in.find('&') != std::string_view::npos);
}

std::size_t DecodeInto(std::string_view in, TextMode mode, char* out) {
  const CharSet& specials = mode == TextMode::kRawText ? kRawTextSpecials : kTextSpecials;
  Writer writer(out);

  std::size_t pos = 0;
  while (pos < in.size()) {
    const std::size_t special = std::min(FindNextOf(in, pos, specials), in.size());
    writer.Put(in.substr(pos, special - pos));
    pos = special;
    if (pos == in.size()) {
      break;
    }

    if (in[pos] == '\r') {
      writer.Put('\n');
      pos += in.substr(pos).starts_with("\r\n") ? 2 : 1;
      continue;
    }

    const std::size_t taken = DecodeReference(in.substr(pos + 1), mode, &writer);
    if (taken == 0) {
      writer.Put('&');
      pos += 1;
    } else {
      pos += 1 + taken;
    }
  }
  return writer.written();
}

void AppendDecoded(std::string_view in, TextMode mode, std::string* out) {
  const std::size_t begin = out->size();
  out->resize(begin + DecodedSizeBound(in.size()));
  out->resize(begin + DecodeInto(in, mode, out->data() + begin));
}

}  // namespace arboris
