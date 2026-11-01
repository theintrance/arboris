/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_ARBORIS_UTILS_STRING_ARENA_HPP_
#define SRC_ARBORIS_UTILS_STRING_ARENA_HPP_

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace arboris {

// Short strings that have to stay where they are: nothing here moves once written, so a
// view handed out stays valid for as long as the arena lives, through moves of the arena
// itself included.
class StringArena {
 public:
  StringArena() = default;
  StringArena(const StringArena&) = delete;
  StringArena& operator=(const StringArena&) = delete;
  StringArena(StringArena&& other) noexcept
      : blocks_(std::move(other.blocks_)),
        cursor_(std::exchange(other.cursor_, nullptr)),
        free_(std::exchange(other.free_, 0)) {}
  StringArena& operator=(StringArena&& other) noexcept {
    blocks_ = std::move(other.blocks_);
    cursor_ = std::exchange(other.cursor_, nullptr);
    free_ = std::exchange(other.free_, 0);
    return *this;
  }
  ~StringArena() = default;

  [[nodiscard]] char* Allocate(std::size_t size) {
    if (size > free_) {
      const std::size_t block_size = std::max(size, kBlockSize);
      // Not make_unique_for_overwrite: libstdc++ 10, which the manylinux wheels build with,
      // lacks it.
      blocks_.emplace_back(new char[block_size]);  // NOLINT(modernize-avoid-c-arrays)
      cursor_ = blocks_.back().get();
      free_ = block_size;
    }
    char* out = cursor_;
    cursor_ += size;
    free_ -= size;
    return out;
  }

  // Returns the unused tail of the last Allocate, for a caller that had to size it by a bound.
  void Shrink(std::size_t unused) {
    cursor_ -= unused;
    free_ += unused;
  }

  [[nodiscard]] std::string_view Append(std::string_view str) {
    char* out = Allocate(str.size());
    std::ranges::copy(str, out);
    return {out, str.size()};
  }

 private:
  static constexpr std::size_t kBlockSize = 4096;

  // Raw bytes, left uninitialised on purpose: every byte is written before it is read.
  std::vector<std::unique_ptr<char[]>> blocks_;  // NOLINT(modernize-avoid-c-arrays)
  char* cursor_ = nullptr;
  std::size_t free_ = 0;
};

}  // namespace arboris

#endif  // SRC_ARBORIS_UTILS_STRING_ARENA_HPP_
