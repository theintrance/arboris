/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_STRING_ARENA_HPP_
#define SRC_UTILS_STRING_ARENA_HPP_

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string_view>
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
  StringArena(StringArena&&) = default;
  StringArena& operator=(StringArena&&) = default;
  ~StringArena() = default;

  [[nodiscard]] char* Allocate(std::size_t size) {
    if (size > free_) {
      const std::size_t block_size = std::max(size, kBlockSize);
      blocks_.push_back(std::make_unique<char[]>(block_size));
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

  std::vector<std::unique_ptr<char[]>> blocks_;
  char* cursor_ = nullptr;
  std::size_t free_ = 0;
};

}  // namespace arboris

#endif  // SRC_UTILS_STRING_ARENA_HPP_
