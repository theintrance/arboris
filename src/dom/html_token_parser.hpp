/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_HTML_TOKEN_PARSER_HPP_
#define SRC_DOM_HTML_TOKEN_PARSER_HPP_

#include <memory>
#include <functional>
#include <string_view>
#include <utility>

#include "dom/token_parser.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

// Forward declaration
class StringPool;

class HtmlTokenParser : public TokenParser {
 public:
  using FeedOpenTokenCallback = std::function<bool(HtmlToken&&, const char*)>;
  using FeedTextTokenCallback = std::function<bool(HtmlTextToken&&)>;
  using FeedCloseTokenCallback = std::function<bool(HtmlCloseToken&&, const char*)>;

  explicit HtmlTokenParser(std::string_view content, std::shared_ptr<StringPool> string_pool)
      : TokenParser(content), string_pool_(std::move(string_pool)) {}

  HtmlTokenParser(const HtmlTokenParser&) = delete;
  HtmlTokenParser& operator=(const HtmlTokenParser&) = delete;
  HtmlTokenParser(HtmlTokenParser&&) = delete;
  HtmlTokenParser& operator=(HtmlTokenParser&&) = delete;

  ~HtmlTokenParser() override = default;

  [[nodiscard]] bool Parse() const override;

  void set_feed_open_token_callback(FeedOpenTokenCallback&& callback) {
    feed_open_token_callback_ = std::move(callback);
  }

  void set_feed_text_token_callback(FeedTextTokenCallback&& callback) {
    feed_text_token_callback_ = std::move(callback);
  }

  void set_feed_close_token_callback(FeedCloseTokenCallback&& callback) {
    feed_close_token_callback_ = std::move(callback);
  }

 private:
  // Delimiter constants for tag parsing
  static constexpr std::string_view kOpenTagDelimiters = " />\t\n\r>";
  static constexpr std::string_view kCloseTagDelimiters = "> \t\n\r";

  [[nodiscard]] std::size_t parseOpenTag(std::size_t begin) const;
  [[nodiscard]] std::size_t parseCloseTag(std::size_t begin) const;
  [[nodiscard]] std::size_t parseTextContent(std::size_t begin) const;

  [[nodiscard]] std::string_view extractTagName(std::size_t* begin, std::string_view delimiters) const;
  [[nodiscard]] bool skipToTagEnd(std::size_t* begin) const;

  std::shared_ptr<StringPool> string_pool_;

  FeedOpenTokenCallback feed_open_token_callback_;
  FeedTextTokenCallback feed_text_token_callback_;
  FeedCloseTokenCallback feed_close_token_callback_;
};

}  // namespace arboris

#endif  // SRC_DOM_HTML_TOKEN_PARSER_HPP_
