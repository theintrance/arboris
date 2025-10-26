/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_STRING_POOL_HPP_
#define SRC_UTILS_STRING_POOL_HPP_

#include <string>

namespace arboris {

class StringPool {
 public:
  explicit StringPool(std::size_t size) {
    pool_.reserve(size);
  }
  StringPool(const StringPool&) = delete;
  StringPool& operator=(const StringPool&) = delete;
  StringPool(StringPool&&) = delete;
  StringPool& operator=(StringPool&&) = delete;
  ~StringPool() = default;

  [[nodiscard]] const char* GetCursor() const {
    return pool_.data() + pool_.size();
  }

  [[nodiscard]] std::string_view Append(std::string_view str) {
    const std::size_t current_size = pool_.size();
    pool_ += str;
    return {pool_.data() + current_size, str.size()};
  }

 private:
  std::string pool_;
};

}  // namespace arboris

#endif  // SRC_UTILS_STRING_POOL_HPP_
