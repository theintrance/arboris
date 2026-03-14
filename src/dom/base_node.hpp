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

// forward declaration
class TagNode;

class BaseNode {
 public:
  explicit BaseNode(NodeType type, std::uint32_t id, std::shared_ptr<TagNode> parent = nullptr)
      : node_type_(type), key_(id), parent_(std::move(parent)) {}

  BaseNode(const BaseNode&) = delete;
  BaseNode& operator=(const BaseNode&) = delete;
  BaseNode(BaseNode&&) = delete;
  BaseNode& operator=(BaseNode&&) = delete;

  virtual ~BaseNode() = default;

  [[nodiscard]] std::uint32_t key() const noexcept {
    return key_;
  }

  [[nodiscard]] NodeType node_type() const noexcept {
    return node_type_;
  }

  [[nodiscard]] std::string_view text_content() const noexcept {
    return text_content_;
  }

  void set_text_content(std::string_view text_content) {
    text_content_ = text_content;
  }

  template <typename T>
  T* As() noexcept {
    static_assert(std::is_base_of_v<BaseNode, T>, "T must be a derived class of BaseNode");
    return (T::kNodeType == node_type_) ? static_cast<T*>(this) : nullptr;
  }

  template <typename T>
  const T* As() const noexcept {
    static_assert(std::is_base_of_v<BaseNode, T>, "T must be a derived class of BaseNode");
    return (T::kNodeType == node_type_) ? static_cast<const T*>(this) : nullptr;
  }

 private:
  const NodeType node_type_;
  const std::uint32_t key_;
  const std::weak_ptr<TagNode> parent_;
  std::string_view text_content_;
};

}  // namespace arboris

#endif  // SRC_DOM_BASE_NODE_HPP_
