/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_HTML_TAG_PROVIDER_HPP_
#define SRC_DOM_HTML_TAG_PROVIDER_HPP_

#include <functional>
#include <string_view>

#include "dom/tag_provider.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

class HtmlTagProvider : public TagProvider {
 public:
  using FeedOpenTokenCallback = std::function<bool(HtmlToken&&)>;
  using FeedTextTokenCallback = std::function<bool(HtmlTextToken&&)>;
  using FeedCloseTokenCallback = std::function<bool(HtmlCloseToken&&)>;

  explicit HtmlTagProvider(std::string_view content);

  HtmlTagProvider(const HtmlTagProvider&) = delete;
  HtmlTagProvider& operator=(const HtmlTagProvider&) = delete;
  HtmlTagProvider(HtmlTagProvider&&) = delete;
  HtmlTagProvider& operator=(HtmlTagProvider&&) = delete;

  ~HtmlTagProvider() override = default;

  bool Parse() override;

  void set_feed_open_token_callback(FeedOpenTokenCallback&& callback);
  void set_feed_text_token_callback(FeedTextTokenCallback&& callback);
  void set_feed_close_token_callback(FeedCloseTokenCallback&& callback);

 private:
  bool parseOpenTag(std::size_t& pos);
  bool parseCloseTag(std::size_t& pos);
  bool parseTextContent(std::size_t& pos);

  Tag getTagFromName(std::string_view tag_name) const;
  bool isVoidTag(Tag tag) const;

  std::string_view extractTagName(std::size_t& pos, const char* delimiters);
  bool skipToTagEnd(std::size_t& pos);

  FeedOpenTokenCallback feed_open_token_callback_;
  FeedTextTokenCallback feed_text_token_callback_;
  FeedCloseTokenCallback feed_close_token_callback_;
};

}  // namespace arboris

#endif  // SRC_DOM_HTML_TAG_PROVIDER_HPP_
