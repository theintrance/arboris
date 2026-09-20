/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

// Bridge between HtmlTokenParser and run_tokenizer_tests.py.
//
// Reads length-prefixed inputs from stdin ("<byte length>\n<bytes>") and writes one
// JSON line per input: {"ok": bool, "tokens": [...]}, where tokens use the
// html5lib-tests tokenizer format (StartTag / EndTag / Character).

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "dom/html_token_parser.hpp"
#include "utils/html_tokens.hpp"
#include "utils/string_pool.hpp"

namespace arboris {
namespace {

// Mirrors the delimiters in HtmlTokenParser. HtmlToken carries only the Tag enum, so the
// raw name is recovered from the token's source span.
// TODO(team): drop this once tokens carry their own tag name.
constexpr std::string_view kTagNameDelimiters = " />\t\n\r>";

std::string_view RawTagName(std::string_view content, const BaseToken& token, std::size_t prefix_len) {
  std::string_view span = content.substr(token.begin_pos, token.end_pos - token.begin_pos);
  span.remove_prefix(prefix_len);
  std::size_t begin = span.find_first_not_of(" \t\n\r\f");
  if (begin == std::string_view::npos) {
    return {};
  }
  span.remove_prefix(begin);
  return span.substr(0, span.find_first_of(kTagNameDelimiters));
}

void AppendJsonString(std::string* out, std::string_view str) {
  out->push_back('"');
  for (char c : str) {
    switch (c) {
      case '"':
        *out += "\\\"";
        break;
      case '\\':
        *out += "\\\\";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", c);
          *out += buf;
        } else {
          out->push_back(c);
        }
    }
  }
  out->push_back('"');
}

std::string Tokenize(std::string_view content) {
  std::string tokens;
  auto append_token = [&tokens](std::string_view kind, std::string_view value, bool with_attrs) {
    if (!tokens.empty()) {
      tokens.push_back(',');
    }
    tokens += "[\"";
    tokens += kind;
    tokens += "\",";
    AppendJsonString(&tokens, value);
    if (with_attrs) {
      tokens += ",{}";  // HtmlTokenParser does not parse attributes yet.
    }
    tokens.push_back(']');
  };

  auto string_pool = std::make_shared<StringPool>(content.size());
  HtmlTokenParser parser(content, string_pool);
  parser.set_feed_open_token_callback([&](HtmlToken&& token, const char*) {
    append_token("StartTag", RawTagName(content, token, 1), true);
    return true;
  });
  parser.set_feed_text_token_callback([&](HtmlTextToken&& token) {
    append_token("Character", token.text_content, false);
    return true;
  });
  parser.set_feed_close_token_callback([&](HtmlCloseToken&& token, const char*) {
    append_token("EndTag", RawTagName(content, token, 2), false);
    return true;
  });

  bool ok = parser.Parse();
  return std::string("{\"ok\":") + (ok ? "true" : "false") + ",\"tokens\":[" + tokens + "]}";
}

}  // anonymous namespace
}  // namespace arboris

int main() {
  std::ios::sync_with_stdio(false);
  std::size_t length = 0;
  while (std::cin >> length) {
    std::cin.get();  // '\n'
    std::string input(length, '\0');
    std::cin.read(input.data(), static_cast<std::streamsize>(length));
    std::cout << arboris::Tokenize(input) << '\n' << std::flush;
  }
  return 0;
}
