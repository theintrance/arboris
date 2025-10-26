/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_BASE_NODE_HPP_
#define SRC_DOM_BASE_NODE_HPP_

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils/assertion.hpp"
#include "utils/html_tokens.hpp"


namespace arboris {

enum class NodeType : std::uint8_t { kTag, kText };

class BaseNode {
 public:
  explicit BaseNode(NodeType type, std::uint32_t id, std::shared_ptr<BaseNode> parent = nullptr)
      : node_type_(type), node_id_(id), parent_(std::move(parent)) {}

  BaseNode(const BaseNode&) = delete;
  BaseNode& operator=(const BaseNode&) = delete;
  BaseNode(BaseNode&&) = delete;
  BaseNode& operator=(BaseNode&&) = delete;

  virtual ~BaseNode() = default;

  [[nodiscard]] std::uint32_t node_id() const noexcept {
    return node_id_;
  }

  [[nodiscard]] NodeType node_type() const noexcept {
    return node_type_;
  }

  [[nodiscard]] std::uint32_t in() const noexcept {
    return in_;
  }

  [[nodiscard]] std::uint32_t out() const noexcept {
    return out_;
  }

  [[nodiscard]] std::string_view text_content() const noexcept {
    return text_content_;
  }

  void set_out(std::uint32_t out) {
    ARBORIS_ASSERT(out > 0, "out must be greater than 0. got " << out);
    out_ = out;
  }

  void set_in(std::uint32_t in) {
    ARBORIS_ASSERT(in > 0, "in must be greater than 0. got " << in);
    in_ = in;
  }

  void set_text_content(std::string_view text_content) {
    ARBORIS_ASSERT(!text_content.empty(), "text_content must not be empty.");
    text_content_ = text_content;
  }

  template <typename T>
  T* As() noexcept {
    static_assert(std::is_base_of<BaseNode, T>::value, "T must be a derived class of BaseNode");
    return (T::kNodeType == node_type_) ? static_cast<T*>(this) : nullptr;
  }

  template <typename T>
  const T* As() const noexcept {
    static_assert(std::is_base_of<BaseNode, T>::value, "T must be a derived class of BaseNode");
    return (T::kNodeType == node_type_) ? static_cast<const T*>(this) : nullptr;
  }

 private:
  const NodeType node_type_;
  const std::uint32_t node_id_;
  const std::weak_ptr<BaseNode> parent_;
  std::string_view text_content_;

  std::uint32_t in_{0};
  std::uint32_t out_{0};
};

}  // namespace arboris

#endif  // SRC_DOM_BASE_NODE_HPP_
