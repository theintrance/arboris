/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_STRING_POOL_HPP_
#define SRC_UTILS_STRING_POOL_HPP_

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string_view>

#include "string/decode.hpp"
#include "utils/assertion.hpp"

namespace arboris {

// The text of one document, in document order and in one buffer, so that the text under
// any node is one view. The buffer is sized for the most the source can decode to and
// never moves, which is what keeps every view handed out valid.
class StringPool {
 public:
  explicit StringPool(std::size_t source_size)
      : capacity_(DecodedSizeBound(source_size)), data_(new char[capacity_]) {}
  StringPool(const StringPool&) = delete;
  StringPool& operator=(const StringPool&) = delete;
  StringPool(StringPool&&) = delete;
  StringPool& operator=(StringPool&&) = delete;
  ~StringPool() = default;

  [[nodiscard]] const char* GetCursor() const {
    return data_.get() + size_;
  }

  [[nodiscard]] std::string_view Append(std::string_view str) {
    ARBORIS_ASSERT(size_ + str.size() <= capacity_, "The string pool is full.");
    std::ranges::copy(str, data_.get() + size_);
    return take(str.size());
  }

  [[nodiscard]] std::string_view AppendDecoded(std::string_view str, TextMode mode) {
    if (!NeedsDecoding(str, mode)) {
      return Append(str);
    }
    ARBORIS_ASSERT(size_ + DecodedSizeBound(str.size()) <= capacity_, "The string pool is full.");
    return take(DecodeInto(str, mode, data_.get() + size_));
  }

 private:
  [[nodiscard]] std::string_view take(std::size_t size) {
    const std::string_view out{data_.get() + size_, size};
    size_ += size;
    return out;
  }

  std::size_t capacity_;
  std::size_t size_ = 0;
  std::unique_ptr<char[]> data_;
};

}  // namespace arboris

#endif  // SRC_UTILS_STRING_POOL_HPP_
