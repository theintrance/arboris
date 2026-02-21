/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_QUERY_OPTIONS_HPP_
#define SRC_UTILS_QUERY_OPTIONS_HPP_

#include <optional>
#include <unordered_set>
#include <string>
#include <string_view>
#include <utility>
#include <functional>

#include "utils/tag.hpp"
#include "utils/html_tokens.hpp"

namespace arboris {

class TextQueryCondition {
 public:
  enum class MatchType {
    kExact,
    kFunction
  };
  explicit TextQueryCondition(std::string_view text)
      : match_type_(MatchType::kExact), text_(text) {}

  explicit TextQueryCondition(std::function<bool(std::string_view)> predicate)
      : match_type_(MatchType::kFunction), predicate_(std::move(predicate)) {}

 private:
  MatchType match_type_;

  // TODO(team): Optimize this. Two instances are not needed at the same time.
  std::string text_;
  std::function<bool(std::string_view)> predicate_;
};

struct QueryOptions {
  std::optional<Tag> tag;
  std::optional<ClassSet> classes;
  std::optional<AttributeMap> attributes;
  std::optional<TextQueryCondition> text;
};

}  // namespace arboris

#endif  // SRC_UTILS_QUERY_OPTIONS_HPP_
