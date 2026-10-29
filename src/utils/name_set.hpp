/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_NAME_SET_HPP_
#define SRC_UTILS_NAME_SET_HPP_

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace arboris {

// Tells a repeated name from a new one in a run that is almost always short. While the run
// holds fewer than kScanLimit names it is scanned, which beats hashing at that size; past
// that the names are hashed, so one element with thousands of classes stays linear.
class NameSet {
 public:
  void Clear() {
    if (!hashed_.empty()) {
      hashed_.clear();
    }
  }

  // True when name is not among earlier, the names of the run so far. The caller then
  // appends it to the run.
  template <std::ranges::sized_range Earlier>
  bool Insert(std::string_view name, const Earlier& earlier) {
    if (std::ranges::size(earlier) < kScanLimit) {
      return std::ranges::find(earlier, name) == std::ranges::end(earlier);
    }
    if (hashed_.empty()) {
      hashed_.insert(std::ranges::begin(earlier), std::ranges::end(earlier));
    }
    return hashed_.insert(name).second;
  }

 private:
  static constexpr std::size_t kScanLimit = 32;

  std::unordered_set<std::string_view> hashed_;
};

}  // namespace arboris

#endif  // SRC_UTILS_NAME_SET_HPP_
