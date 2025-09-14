/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_NODE_HPP_
#define SRC_DOM_NODE_HPP_

#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

#include "utils/html_tokens.hpp"

namespace arboris {

class Node {
 public:
  explicit Node(
    std::uint32_t id,
    HtmlToken&& html_token,
    const std::shared_ptr<Node> parent = nullptr)
      : id_(id), parent_(parent), html_token_(std::move(html_token)) {}

  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
  Node(Node&&) = delete;
  Node& operator=(Node&&) = delete;

  virtual ~Node() = default;

  inline std::uint32_t id() const noexcept {
      return id_;
  }

  inline std::uint32_t in() const noexcept {
    return in_;
  }

  inline std::uint32_t out() const noexcept {
    return out_;
  }

  inline const HtmlToken& html_token() const noexcept {
    return html_token_;
  }

  inline std::string_view text_content() const noexcept {
    return text_content_;
  }

  inline void set_out(std::uint32_t out) {
    // TODO(Jayden): Call assertion here
    out_ = out;
  }

  inline void set_in(std::uint32_t in) {
    // TODO(Jayden): Call assertion here
    in_ = in;
  }

  inline void set_text_content(std::string_view text_content) {
    // TODO(Jayden): Call assertion here
    text_content_ = text_content;
  }

 private:
  const std::uint32_t id_;
  const std::shared_ptr<Node> parent_;

  const HtmlToken html_token_;
  std::string_view text_content_;

  std::uint32_t in_{0};
  std::uint32_t out_{0};
};

}  // namespace arboris

#endif  // SRC_DOM_NODE_HPP_
