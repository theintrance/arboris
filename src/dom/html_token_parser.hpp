/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_HTML_TOKEN_PARSER_HPP_
#define SRC_DOM_HTML_TOKEN_PARSER_HPP_

#include <cstddef>
#include <memory>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/token_parser.hpp"
#include "utils/html_tokens.hpp"
#include "utils/string_arena.hpp"

namespace arboris {

// Forward declaration
class StringPool;

class HtmlTokenParser : public TokenParser {
 public:
  // An open tag comes with its attributes. They point into the content, but the array that
  // holds them is reused for the next tag, so a callback that keeps them copies them out.
  using FeedOpenTokenCallback = std::function<bool(HtmlToken&&, std::span<const Attribute>, const char*)>;
  using FeedTextTokenCallback = std::function<bool(HtmlTextToken&&)>;
  using FeedCloseTokenCallback = std::function<bool(HtmlCloseToken&&, const char*)>;
  using FeedCommentTokenCallback = std::function<bool(HtmlCommentToken&&)>;
  using FeedDoctypeTokenCallback = std::function<bool(HtmlDoctypeToken&&)>;

  explicit HtmlTokenParser(std::string_view content, std::shared_ptr<StringPool> string_pool)
      : TokenParser(content), string_pool_(std::move(string_pool)) {}

  HtmlTokenParser(const HtmlTokenParser&) = delete;
  HtmlTokenParser& operator=(const HtmlTokenParser&) = delete;
  HtmlTokenParser(HtmlTokenParser&&) = delete;
  HtmlTokenParser& operator=(HtmlTokenParser&&) = delete;

  ~HtmlTokenParser() override = default;

  [[nodiscard]] bool Parse() override;

  // Where the lower-cased spellings of tag and attribute names live. Token views into it
  // outlive the parser only if the caller takes it.
  [[nodiscard]] StringArena ReleaseNames() {
    return std::move(names_);
  }

  // After Parse() returns false: the start of the token it stopped at.
  [[nodiscard]] std::size_t error_pos() const noexcept {
    return error_pos_;
  }

  void set_feed_open_token_callback(FeedOpenTokenCallback&& callback) {
    feed_open_token_callback_ = std::move(callback);
  }

  void set_feed_text_token_callback(FeedTextTokenCallback&& callback) {
    feed_text_token_callback_ = std::move(callback);
  }

  void set_feed_close_token_callback(FeedCloseTokenCallback&& callback) {
    feed_close_token_callback_ = std::move(callback);
  }

  void set_feed_comment_token_callback(FeedCommentTokenCallback&& callback) {
    feed_comment_token_callback_ = std::move(callback);
  }

  void set_feed_doctype_token_callback(FeedDoctypeTokenCallback&& callback) {
    feed_doctype_token_callback_ = std::move(callback);
  }

 private:
  // Delimiter constants for tag parsing
  static constexpr std::string_view kOpenTagDelimiters = " />\t\n\f\r>";
  static constexpr std::string_view kCloseTagDelimiters = "> \t\n\f\r";

  // An attribute name runs up to whitespace, '/', '>' or '='. An unquoted value runs up to
  // whitespace or '>' only, so a '/' inside it is part of the value.
  static constexpr std::string_view kAttributeNameDelimiters = " \t\n\f\r/>=";
  static constexpr std::string_view kUnquotedValueDelimiters = " \t\n\f\r>";

  struct RawTextElement {
    Tag tag;
    std::string_view name;
  };

  [[nodiscard]] std::size_t parseOpenTag(std::size_t begin, std::vector<Attribute>* attributes,
                                         std::optional<RawTextElement>* raw_text);
  [[nodiscard]] std::size_t parseCloseTag(std::size_t begin);
  [[nodiscard]] std::size_t parseTextContent(std::size_t begin) const;
  [[nodiscard]] std::size_t parseMarkupDeclaration(std::size_t begin);
  [[nodiscard]] std::size_t parseComment(std::size_t begin) const;
  [[nodiscard]] std::size_t parseBogusComment(std::size_t begin, std::size_t data_begin) const;
  [[nodiscard]] std::size_t parseDoctype(std::size_t begin);

  [[nodiscard]] std::size_t feedCloseTag(std::size_t begin, std::size_t name_end, Tag tag, std::string_view name) const;
  [[nodiscard]] std::size_t feedText(std::size_t begin, std::size_t end) const;
  [[nodiscard]] std::size_t feedComment(std::size_t begin, std::size_t end, std::string_view data) const;

  [[nodiscard]] std::string_view extractTagName(std::size_t* begin, std::string_view delimiters) const;
  [[nodiscard]] std::string_view lowerCased(std::string_view name);
  [[nodiscard]] bool parseAttributes(std::size_t* begin, std::vector<Attribute>* attributes, bool* self_closing);
  [[nodiscard]] bool skipToTagEnd(std::size_t* begin) const;
  [[nodiscard]] std::size_t findRawTextEnd(std::size_t begin, std::string_view tag_name) const;
  [[nodiscard]] std::optional<std::string_view> readDoctypeId(std::size_t* begin, bool* well_formed) const;

  std::shared_ptr<StringPool> string_pool_;
  StringArena names_;
  std::size_t error_pos_ = 0;

  FeedOpenTokenCallback feed_open_token_callback_;
  FeedTextTokenCallback feed_text_token_callback_;
  FeedCloseTokenCallback feed_close_token_callback_;
  FeedCommentTokenCallback feed_comment_token_callback_;
  FeedDoctypeTokenCallback feed_doctype_token_callback_;
};

}  // namespace arboris

#endif  // SRC_DOM_HTML_TOKEN_PARSER_HPP_
