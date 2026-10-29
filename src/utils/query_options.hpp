/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_QUERY_OPTIONS_HPP_
#define SRC_UTILS_QUERY_OPTIONS_HPP_

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "utils/html_tokens.hpp"
#include "utils/tag.hpp"

namespace arboris {

// Holds for a node when its text_content(), the text of the node and of everything below
// it as written in the source, equals the given text or satisfies the predicate.
class TextQueryCondition {
 public:
  using Predicate = std::function<bool(std::string_view)>;

  explicit TextQueryCondition(std::string_view text) : condition_(std::string(text)) {}
  explicit TextQueryCondition(Predicate predicate) : condition_(std::move(predicate)) {}

  [[nodiscard]] bool Matches(std::string_view text) const {
    if (const auto* exact = std::get_if<std::string>(&condition_)) {
      return *exact == text;
    }
    return std::get<Predicate>(condition_)(text);
  }

 private:
  std::variant<std::string, Predicate> condition_;
};

// Every condition given has to hold. With none given, every element matches.
struct QueryOptions {
  std::optional<Tag> tag;
  std::optional<ClassSet> classes;
  std::optional<AttributeMap> attributes;
  std::optional<TextQueryCondition> text;
};

}  // namespace arboris

#endif  // SRC_UTILS_QUERY_OPTIONS_HPP_
