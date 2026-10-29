/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "encoding/encoding.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "string/string.hpp"

#if defined(ARBORIS_HAVE_ICONV)
#include <iconv.h>
#include <cerrno>
#endif

namespace arboris {

namespace {

constexpr std::string_view kUtf8 = "utf-8";
constexpr std::string_view kUtf16Le = "utf-16le";
constexpr std::string_view kUtf16Be = "utf-16be";
constexpr std::size_t kPrescanBytes = 1024;
constexpr std::string_view kReplacementCharacter = "\xef\xbf\xbd";

struct Label {
  std::string_view label;
  std::string_view name;
};

// The Encoding Standard's labels for the encodings pages are found in, by their name.
constexpr std::array kLabels = std::to_array<Label>({
    {"unicode-1-1-utf-8", kUtf8}, {"unicode11utf8", kUtf8}, {"unicode20utf8", kUtf8}, {"utf-8", kUtf8},
    {"utf8", kUtf8}, {"x-unicode20utf8", kUtf8},
    {"utf-16", kUtf16Le}, {"utf-16le", kUtf16Le}, {"ucs-2", kUtf16Le}, {"unicode", kUtf16Le},
    {"unicodefeff", kUtf16Le}, {"csunicode", kUtf16Le}, {"iso-10646-ucs-2", kUtf16Le},
    {"utf-16be", kUtf16Be}, {"unicodefffe", kUtf16Be},
    {"ansi_x3.4-1968", "windows-1252"}, {"ascii", "windows-1252"}, {"cp1252", "windows-1252"},
    {"cp819", "windows-1252"}, {"csisolatin1", "windows-1252"}, {"ibm819", "windows-1252"},
    {"iso-8859-1", "windows-1252"}, {"iso-ir-100", "windows-1252"}, {"iso8859-1", "windows-1252"},
    {"iso88591", "windows-1252"}, {"iso_8859-1", "windows-1252"}, {"iso_8859-1:1987", "windows-1252"},
    {"l1", "windows-1252"}, {"latin1", "windows-1252"}, {"us-ascii", "windows-1252"},
    {"windows-1252", "windows-1252"}, {"x-cp1252", "windows-1252"},
    {"cseuckr", "euc-kr"}, {"csksc56011987", "euc-kr"}, {"euc-kr", "euc-kr"}, {"iso-ir-149", "euc-kr"},
    {"korean", "euc-kr"}, {"ks_c_5601-1987", "euc-kr"}, {"ks_c_5601-1989", "euc-kr"}, {"ksc5601", "euc-kr"},
    {"ksc_5601", "euc-kr"}, {"windows-949", "euc-kr"},
    {"csshiftjis", "shift_jis"}, {"ms932", "shift_jis"}, {"ms_kanji", "shift_jis"}, {"shift-jis", "shift_jis"},
    {"shift_jis", "shift_jis"}, {"sjis", "shift_jis"}, {"windows-31j", "shift_jis"}, {"x-sjis", "shift_jis"},
    {"cseucpkdfmtjapanese", "euc-jp"}, {"euc-jp", "euc-jp"}, {"x-euc-jp", "euc-jp"},
    {"csiso2022jp", "iso-2022-jp"}, {"iso-2022-jp", "iso-2022-jp"},
    {"chinese", "gbk"}, {"csgb2312", "gbk"}, {"csiso58gb231280", "gbk"}, {"gb2312", "gbk"}, {"gb_2312", "gbk"},
    {"gb_2312-80", "gbk"}, {"gbk", "gbk"}, {"iso-ir-58", "gbk"}, {"x-gbk", "gbk"},
    {"gb18030", "gb18030"},
    {"big5", "big5"}, {"big5-hkscs", "big5"}, {"cn-big5", "big5"}, {"csbig5", "big5"}, {"x-x-big5", "big5"},
    {"cskoi8r", "koi8-r"}, {"koi", "koi8-r"}, {"koi8", "koi8-r"}, {"koi8-r", "koi8-r"}, {"koi8_r", "koi8-r"},
    {"koi8-ru", "koi8-u"}, {"koi8-u", "koi8-u"},
    {"dos-874", "windows-874"}, {"iso-8859-11", "windows-874"}, {"tis-620", "windows-874"},
    {"windows-874", "windows-874"},
    {"cp1250", "windows-1250"}, {"windows-1250", "windows-1250"}, {"x-cp1250", "windows-1250"},
    {"cp1251", "windows-1251"}, {"windows-1251", "windows-1251"}, {"x-cp1251", "windows-1251"},
    {"cp1253", "windows-1253"}, {"windows-1253", "windows-1253"}, {"x-cp1253", "windows-1253"},
    {"cp1254", "windows-1254"}, {"iso-8859-9", "windows-1254"}, {"latin5", "windows-1254"},
    {"windows-1254", "windows-1254"}, {"x-cp1254", "windows-1254"},
    {"cp1255", "windows-1255"}, {"windows-1255", "windows-1255"}, {"x-cp1255", "windows-1255"},
    {"cp1256", "windows-1256"}, {"windows-1256", "windows-1256"}, {"x-cp1256", "windows-1256"},
    {"cp1257", "windows-1257"}, {"windows-1257", "windows-1257"}, {"x-cp1257", "windows-1257"},
    {"cp1258", "windows-1258"}, {"windows-1258", "windows-1258"}, {"x-cp1258", "windows-1258"},
    {"iso-8859-2", "iso-8859-2"}, {"iso8859-2", "iso-8859-2"}, {"latin2", "iso-8859-2"}, {"l2", "iso-8859-2"},
    {"iso-8859-3", "iso-8859-3"}, {"latin3", "iso-8859-3"},
    {"iso-8859-4", "iso-8859-4"}, {"latin4", "iso-8859-4"},
    {"iso-8859-5", "iso-8859-5"}, {"cyrillic", "iso-8859-5"},
    {"iso-8859-6", "iso-8859-6"}, {"arabic", "iso-8859-6"},
    {"iso-8859-7", "iso-8859-7"}, {"greek", "iso-8859-7"},
    {"iso-8859-8", "iso-8859-8"}, {"hebrew", "iso-8859-8"},
    {"iso-8859-10", "iso-8859-10"}, {"latin6", "iso-8859-10"},
    {"iso-8859-13", "iso-8859-13"}, {"iso-8859-14", "iso-8859-14"},
    {"iso-8859-15", "iso-8859-15"}, {"latin9", "iso-8859-15"}, {"l9", "iso-8859-15"},
    {"iso-8859-16", "iso-8859-16"},
    {"macintosh", "macintosh"}, {"mac", "macintosh"}, {"x-mac-roman", "macintosh"},
});

// What iconv calls the encodings whose Encoding Standard name it does not take as-is.
// The Standard's euc-kr, shift_jis and gbk are the Windows supersets of those encodings.
constexpr std::array kIconvNames = std::to_array<Label>({
    {"euc-kr", "CP949"}, {"shift_jis", "CP932"}, {"gbk", "CP936"}, {"windows-874", "CP874"},
    {"macintosh", "MACINTOSH"},
});

char ToAsciiLower(char c) {
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

std::string AsciiLowerCase(std::string_view text) {
  std::string out(text);
  std::ranges::transform(out, out.begin(), ToAsciiLower);
  return out;
}

std::size_t FindIgnoringCase(std::string_view haystack, std::string_view needle, std::size_t from) {
  for (std::size_t pos = from; pos + needle.size() <= haystack.size(); ++pos) {
    if (std::ranges::equal(haystack.substr(pos, needle.size()), needle, {}, ToAsciiLower, ToAsciiLower)) {
      return pos;
    }
  }
  return std::string_view::npos;
}

std::string_view Trim(std::string_view text) {
  const std::size_t begin = SkipHtmlWhitespace(text, 0);
  std::size_t end = text.size();
  while (end > begin && kHtmlWhitespaceSet.Contains(text[end - 1])) {
    --end;
  }
  return text.substr(begin, end - begin);
}

// The charset named in a Content-Type value such as `text/html; charset=euc-kr`.
std::optional<std::string_view> CharsetOfContentType(std::string_view content) {
  const std::size_t at = FindIgnoringCase(content, "charset", 0);
  if (at == std::string_view::npos) {
    return std::nullopt;
  }
  std::size_t pos = SkipHtmlWhitespace(content, at + 7);
  if (pos >= content.size() || content[pos] != '=') {
    return std::nullopt;
  }
  pos = SkipHtmlWhitespace(content, pos + 1);
  if (pos < content.size() && (content[pos] == '"' || content[pos] == '\'')) {
    const std::size_t end = content.find(content[pos], pos + 1);
    return end == std::string_view::npos ? std::nullopt : std::make_optional(content.substr(pos + 1, end - pos - 1));
  }
  const std::size_t end = std::min(FindNextOf(content, pos, CharSet{" \t\n\f\r;"}), content.size());
  return content.substr(pos, end - pos);
}

// The attributes of one <meta ...> tag starting at `begin`, read the way the prescan
// algorithm does, until the '>' that ends it. Returns the charset the tag declares.
std::optional<std::string> CharsetOfMeta(std::string_view head, std::size_t begin) {
  static constexpr CharSet kNameEnd{" \t\n\f\r/>="};
  static constexpr CharSet kUnquotedEnd{" \t\n\f\r>"};

  std::optional<std::string_view> charset;
  std::optional<std::string_view> content;
  bool content_type = false;

  std::size_t pos = begin;
  while (pos < head.size() && head[pos] != '>') {
    pos = SkipAll(head, pos, CharSet{" \t\n\f\r/"});
    if (pos >= head.size() || head[pos] == '>') {
      break;
    }
    const std::size_t name_begin = pos;
    pos = std::min(FindNextOf(head, pos + 1, kNameEnd), head.size());
    const std::string name = AsciiLowerCase(head.substr(name_begin, pos - name_begin));

    std::string_view value;
    pos = SkipHtmlWhitespace(head, pos);
    if (pos < head.size() && head[pos] == '=') {
      pos = SkipHtmlWhitespace(head, pos + 1);
      if (pos < head.size() && (head[pos] == '"' || head[pos] == '\'')) {
        const std::size_t end = head.find(head[pos], pos + 1);
        if (end == std::string_view::npos) {
          return std::nullopt;
        }
        value = head.substr(pos + 1, end - pos - 1);
        pos = end + 1;
      } else {
        const std::size_t end = std::min(FindNextOf(head, pos, kUnquotedEnd), head.size());
        value = head.substr(pos, end - pos);
        pos = end;
      }
    }

    if (name == "charset" && !charset) {
      charset = value;
    } else if (name == "content" && !content) {
      content = value;
    } else if (name == "http-equiv" && AsciiLowerCase(Trim(value)) == "content-type") {
      content_type = true;
    }
  }

  if (charset) {
    return std::string(Trim(*charset));
  }
  if (content_type && content) {
    if (const auto found = CharsetOfContentType(*content)) {
      return std::string(Trim(*found));
    }
  }
  return std::nullopt;
}

std::optional<std::string> PrescanMeta(std::string_view bytes) {
  const std::string_view head = bytes.substr(0, kPrescanBytes);
  for (std::size_t pos = head.find('<'); pos != std::string_view::npos; pos = head.find('<', pos + 1)) {
    if (head.substr(pos).starts_with("<!--")) {
      const std::size_t end = head.find("-->", pos + 4);
      if (end == std::string_view::npos) {
        return std::nullopt;
      }
      pos = end + 2;
      continue;
    }
    if (FindIgnoringCase(head.substr(pos, 6), "<meta", 0) == 0 && pos + 5 < head.size() &&
        CharSet{" \t\n\f\r/"}.Contains(head[pos + 5])) {
      if (auto charset = CharsetOfMeta(head, pos + 5)) {
        if (auto name = CanonicalEncoding(*charset)) {
          return name;
        }
      }
    }
  }
  return std::nullopt;
}

void AppendUtf8(char32_t code_point, std::string* out) {
  if (code_point < 0x80) {
    out->push_back(static_cast<char>(code_point));
  } else if (code_point < 0x800) {
    out->push_back(static_cast<char>(0xC0 | (code_point >> 6)));
    out->push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
  } else if (code_point < 0x10000) {
    out->push_back(static_cast<char>(0xE0 | (code_point >> 12)));
    out->push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
    out->push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
  } else {
    out->push_back(static_cast<char>(0xF0 | (code_point >> 18)));
    out->push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
    out->push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
    out->push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
  }
}

// A lone surrogate and a trailing odd byte each become U+FFFD, as the Encoding Standard
// decodes them.
std::string Utf16ToUtf8(std::string_view bytes, bool big_endian) {
  std::string out;
  out.reserve(bytes.size() * 3 / 2);

  const auto unit_at = [&](std::size_t i) -> char32_t {
    const auto lo = static_cast<unsigned char>(bytes[i + (big_endian ? 1 : 0)]);
    const auto hi = static_cast<unsigned char>(bytes[i + (big_endian ? 0 : 1)]);
    return static_cast<char32_t>((hi << 8) | lo);
  };

  std::size_t i = 0;
  for (; i + 1 < bytes.size(); i += 2) {
    const char32_t unit = unit_at(i);
    if (unit >= 0xD800 && unit <= 0xDBFF && i + 3 < bytes.size()) {
      const char32_t low = unit_at(i + 2);
      if (low >= 0xDC00 && low <= 0xDFFF) {
        AppendUtf8(0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00), &out);
        i += 2;
        continue;
      }
    }
    if (unit >= 0xD800 && unit <= 0xDFFF) {
      out += kReplacementCharacter;
    } else {
      AppendUtf8(unit, &out);
    }
  }
  if (i < bytes.size()) {
    out += kReplacementCharacter;
  }
  return out;
}

#if defined(ARBORIS_HAVE_ICONV)
// Bytes iconv cannot read become U+FFFD and decoding goes on from the next byte.
std::optional<std::string> IconvToUtf8(std::string_view bytes, std::string_view encoding) {
  std::string from(encoding);
  for (const auto& [name, iconv_name] : kIconvNames) {
    if (name == encoding) {
      from = iconv_name;
    }
  }

  const iconv_t converter = iconv_open("UTF-8", from.c_str());
  if (converter == reinterpret_cast<iconv_t>(-1)) {
    return std::nullopt;
  }

  std::string out;
  out.resize(bytes.size() * 4 + 4);
  char* in_ptr = const_cast<char*>(bytes.data());  // NOLINT(cppcoreguidelines-pro-type-const-cast)
  std::size_t in_left = bytes.size();
  char* out_ptr = out.data();
  std::size_t out_left = out.size();

  while (in_left > 0) {
    if (iconv(converter, &in_ptr, &in_left, &out_ptr, &out_left) != static_cast<std::size_t>(-1)) {
      break;
    }
    if (errno == E2BIG) {
      const std::size_t used = out.size() - out_left;
      out.resize(out.size() * 2);
      out_ptr = out.data() + used;
      out_left = out.size() - used;
      continue;
    }
    if (out_left < kReplacementCharacter.size()) {
      const std::size_t used = out.size() - out_left;
      out.resize(out.size() * 2);
      out_ptr = out.data() + used;
      out_left = out.size() - used;
    }
    std::ranges::copy(kReplacementCharacter, out_ptr);
    out_ptr += kReplacementCharacter.size();
    out_left -= kReplacementCharacter.size();
    if (errno == EINVAL) {
      break;
    }
    ++in_ptr;
    --in_left;
  }
  iconv_close(converter);

  out.resize(out.size() - out_left);
  return out;
}
#endif

}  // anonymous namespace

std::optional<std::string> CanonicalEncoding(std::string_view label) {
  const std::string lowered = AsciiLowerCase(Trim(label));
  for (const auto& [known, name] : kLabels) {
    if (known == lowered) {
      return std::string(name);
    }
  }
  return std::nullopt;
}

std::optional<std::string> SniffEncoding(std::string_view bytes) {
  if (bytes.starts_with("\xef\xbb\xbf")) {
    return std::string(kUtf8);
  }
  if (bytes.starts_with("\xfe\xff")) {
    return std::string(kUtf16Be);
  }
  if (bytes.starts_with("\xff\xfe")) {
    return std::string(kUtf16Le);
  }
  return PrescanMeta(bytes);
}

std::optional<std::string> ToUtf8(std::string_view bytes, std::string_view encoding) {
  if (encoding == kUtf8) {
    return std::string(bytes);
  }
  if (encoding == kUtf16Le || encoding == kUtf16Be) {
    return Utf16ToUtf8(bytes, encoding == kUtf16Be);
  }
#if defined(ARBORIS_HAVE_ICONV)
  return IconvToUtf8(bytes, encoding);
#else
  return std::nullopt;
#endif
}

DecodedDocument DecodeDocument(std::string_view bytes, std::string_view encoding_label) {
  std::optional<std::string> encoding;
  if (bytes.starts_with("\xef\xbb\xbf")) {
    encoding = kUtf8;
    bytes.remove_prefix(3);
  } else if (bytes.starts_with("\xfe\xff") || bytes.starts_with("\xff\xfe")) {
    encoding = bytes.starts_with("\xfe\xff") ? kUtf16Be : kUtf16Le;
    bytes.remove_prefix(2);
  } else if (!encoding_label.empty()) {
    encoding = CanonicalEncoding(encoding_label);
    if (!encoding) {
      return {.utf8 = std::string(bytes), .encoding = std::string(kUtf8),
              .unsupported_encoding = std::string(encoding_label)};
    }
  } else {
    encoding = PrescanMeta(bytes);
  }

  const std::string name = encoding.value_or(std::string(kUtf8));
  if (auto utf8 = ToUtf8(bytes, name)) {
    return {.utf8 = std::move(*utf8), .encoding = name, .unsupported_encoding = std::nullopt};
  }
  return {.utf8 = std::string(bytes), .encoding = std::string(kUtf8), .unsupported_encoding = name};
}

}  // namespace arboris
