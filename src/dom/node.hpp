/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_NODE_HPP_
#define SRC_DOM_NODE_HPP_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace arboris {

class Node {
 public:
  explicit Node(const std::string& tag, std::uint32_t id, const Node* parent = nullptr)
      : tag_(tag), id_(id), parent_(parent) {}

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

  inline void set_out(std::uint32_t out) {
    // TODO(Jayden): Call assertion here
    out_ = out;
  }

  inline void set_in(std::uint32_t in) {
    // TODO(Jayden): Call assertion here
    in_ = in;
  }

 private:
  const std::string tag_;
  const std::uint32_t id_;
  const Node* parent_;

  // TODO(Team): Optimize the attribute storage (std::string -> std::string_view)
  std::unordered_map<std::string, std::vector<std::string>> attrs_;
  std::vector<const Node*> children_;

  std::uint32_t in_{0};
  std::uint32_t out_{0};
};

}  // namespace arboris

#endif  // SRC_DOM_NODE_HPP_
