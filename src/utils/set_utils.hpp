/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_SET_UTILS_HPP_
#define SRC_UTILS_SET_UTILS_HPP_

#include <algorithm>
#include <iterator>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

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

// The superset as a plain range rather than a set. A short contiguous range is faster to
// scan than to hash, which is what a node's own classes are. The set-to-set overload above
// is the more specialised one, so a set on the right still hashes.
template <typename T, std::ranges::input_range Range>
bool IsSubset(const std::unordered_set<T>& subset, const Range& super_set) {
  if constexpr (std::ranges::sized_range<Range>) {
    if (subset.size() > std::ranges::size(super_set)) {
      return false;
    }
  }

  return std::all_of(subset.begin(), subset.end(), [&super_set](const auto& value) {
    return std::ranges::find(super_set, value) != std::ranges::end(super_set);
  });
}

}  // namespace arboris

#endif  // SRC_UTILS_SET_UTILS_HPP_
