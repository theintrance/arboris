/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_SET_UTILS_HPP_
#define SRC_UTILS_SET_UTILS_HPP_

#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "utils/html_tokens.hpp"

namespace arboris {

template <typename T>
bool IsSubset(const std::unordered_set<T>& subset,
              const std::unordered_set<T>& super_set) {
  if (subset.size() > super_set.size()) {
    return false;
  }

  for (const auto& x : subset) {
    // std::unordered_set::contains is available in C++20
    if (!super_set.contains(x)) {
      return false;
    }
  }
  return true;
}

template <typename T, typename U>
bool IsSubset(const std::unordered_map<T, std::unordered_set<U>>& subset,
              const std::unordered_map<T, std::unordered_set<U>>& super_set) {
  for (const auto& [key, value_set] : subset) {
    auto it = super_set.find(key);
    if (it == super_set.end()) {
      return false;
    }
    if (!IsSubset(value_set, it->second)) {
      return false;
    }
  }
  return true;
}

// A node carries a handful of classes in one contiguous run, so scanning it beats hashing
// each name. Same for attributes below.
inline bool IsSubset(const ClassSet& wanted, std::span<const std::string_view> classes) {
  if (wanted.size() > classes.size()) {
    return false;
  }

  return std::all_of(wanted.begin(), wanted.end(), [classes](std::string_view name) {
    return std::find(classes.begin(), classes.end(), name) != classes.end();
  });
}

// An attribute named with no wanted value matches on the name alone, which is how
// `[disabled]` style conditions read.
inline bool IsSubset(const AttributeMap& wanted, std::span<const Attribute> attributes) {
  for (const auto& [name, values] : wanted) {
    const auto found = std::find_if(
        attributes.begin(), attributes.end(),
        [&name](const Attribute& attribute) { return attribute.name == name; });

    if (found == attributes.end()) {
      return false;
    }
    if (!values.empty() && !values.contains(std::string(found->value))) {
      return false;
    }
  }
  return true;
}

}  // namespace arboris

#endif  // SRC_UTILS_SET_UTILS_HPP_
