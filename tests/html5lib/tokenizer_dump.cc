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

#include "dom/html_token_parser.hpp"
#include "utils/html_tokens.hpp"
#include "utils/string_pool.hpp"

namespace arboris {
namespace {

// Tokens carry their lower-cased name. CI also builds this file against the base branch,
// whose tokens may carry only the Tag, so the raw name is then read from the source span.
// TODO(team): drop RawTagName once develop's tokens have a name (#69).
constexpr std::string_view kTagNameDelimiters = " />\t\n\f\r>";

// The open-tag callback hands over the tag's attributes. CI also builds this file against
// the base branch, whose callback may not pass them yet, so either shape is taken.
// TODO(team): drop the second overload once develop's callback passes attributes.
std::span<const Attribute> AttributesOf(std::span<const Attribute> attributes, const char*) {
  return attributes;
}

std::span<const Attribute> AttributesOf(const char*) {
  return {};
}

// Likewise for the self-closing flag and the comment and DOCTYPE callbacks.
// TODO(team): use them directly once develop has them.
template <typename Token>
bool IsSelfClosing(const Token& token) {
  if constexpr (requires { token.is_self_closing; }) {
    return token.is_self_closing;
  } else {
    return false;
  }
}

template <typename Parser, typename Callback>
void SetCommentCallback(Parser* parser, Callback&& callback) {
  if constexpr (requires { parser->set_feed_comment_token_callback(callback); }) {
    parser->set_feed_comment_token_callback(std::forward<Callback>(callback));
  }
}

template <typename Parser, typename Callback>
void SetDoctypeCallback(Parser* parser, Callback&& callback) {
  if constexpr (requires { parser->set_feed_doctype_token_callback(callback); }) {
    parser->set_feed_doctype_token_callback(std::forward<Callback>(callback));
  }
}

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

template <typename Token>
std::string_view TagName(std::string_view content, const Token& token, std::size_t prefix_len) {
  if constexpr (requires { token.name; }) {
    return token.name;
  } else {
    return RawTagName(content, token, prefix_len);
  }
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
  parser.set_feed_open_token_callback([&](HtmlToken&& token, auto... rest) {
    append_token("StartTag", TagName(content, token, 1), AttributesOf(rest...), IsSelfClosing(token));
    return true;
  });
  parser.set_feed_text_token_callback([&](HtmlTextToken&& token) {
    append_token("Character", token.text_content, std::nullopt);
    return true;
  });
  parser.set_feed_close_token_callback([&](HtmlCloseToken&& token, const char*) {
    append_token("EndTag", TagName(content, token, 2), std::nullopt);
    return true;
  });
  SetCommentCallback(&parser, [&](auto&& token) {
    append_token("Comment", token.data, std::nullopt);
    return true;
  });
  SetDoctypeCallback(&parser, [&](auto&& token) {
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
