/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

// Bridge between HtmlTokenParser and run_tokenizer_tests.py.
//
// Reads length-prefixed inputs from stdin ("<byte length>\n<bytes>") and writes one
// JSON line per input: {"ok": bool, "tokens": [...]}, where tokens use the
// html5lib-tests tokenizer format (StartTag / EndTag / Character / Comment / DOCTYPE).

#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "arboris/dom/html_token_parser.hpp"
#include "arboris/utils/html_tokens.hpp"
#include "arboris/utils/string_pool.hpp"

namespace arboris {
namespace {

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

// No name repeats within a tag: the parser keeps the first of a repeated one.
void AppendJsonObject(std::string* out, std::span<const Attribute> attributes) {
  out->push_back('{');
  for (std::size_t i = 0; i < attributes.size(); ++i) {
    if (i > 0) {
      out->push_back(',');
    }
    AppendJsonString(out, attributes[i].name);
    out->push_back(':');
    AppendJsonString(out, attributes[i].value);
  }
  out->push_back('}');
}

void AppendJsonStringOrNull(std::string* out, std::optional<std::string_view> str) {
  if (str) {
    AppendJsonString(out, *str);
  } else {
    *out += "null";
  }
}

std::string Tokenize(std::string_view content) {
  std::string tokens;
  auto begin_token = [&tokens](std::string_view kind) {
    if (!tokens.empty()) {
      tokens.push_back(',');
    }
    tokens += "[\"";
    tokens += kind;
    tokens += "\"";
  };
  auto append_token = [&](std::string_view kind, std::string_view value,
                          std::optional<std::span<const Attribute>> attributes, bool self_closing = false) {
    begin_token(kind);
    tokens.push_back(',');
    AppendJsonString(&tokens, value);
    if (attributes) {
      tokens.push_back(',');
      AppendJsonObject(&tokens, *attributes);
    }
    if (self_closing) {
      tokens += ",true";
    }
    tokens.push_back(']');
  };

  auto string_pool = std::make_shared<StringPool>(content.size());
  HtmlTokenParser parser(content, string_pool);
  parser.set_feed_open_token_callback([&](HtmlToken&& token, std::span<const Attribute> attributes, const char*) {
    append_token("StartTag", token.name, attributes, token.is_self_closing);
    return true;
  });
  parser.set_feed_text_token_callback([&](HtmlTextToken&& token) {
    append_token("Character", token.text_content, std::nullopt);
    return true;
  });
  parser.set_feed_close_token_callback([&](HtmlCloseToken&& token, const char*) {
    append_token("EndTag", token.name, std::nullopt);
    return true;
  });
  parser.set_feed_comment_token_callback([&](HtmlCommentToken&& token) {
    append_token("Comment", token.data, std::nullopt);
    return true;
  });
  parser.set_feed_doctype_token_callback([&](HtmlDoctypeToken&& token) {
    begin_token("DOCTYPE");
    tokens.push_back(',');
    AppendJsonStringOrNull(&tokens, token.name.empty() ? std::nullopt : std::make_optional(token.name));
    tokens.push_back(',');
    AppendJsonStringOrNull(&tokens, token.public_id);
    tokens.push_back(',');
    AppendJsonStringOrNull(&tokens, token.system_id);
    tokens += token.force_quirks ? ",false]" : ",true]";
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
