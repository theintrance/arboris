/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TAG_PROVIDER_HPP_
#define SRC_DOM_TAG_PROVIDER_HPP_

#include <string_view>

namespace arboris {

class TagProvider {
 public:
  explicit TagProvider(std::string_view content) : content_(content) {}

  TagProvider(const TagProvider&) = delete;
  TagProvider& operator=(const TagProvider&) = delete;
  TagProvider(TagProvider&&) = delete;
  TagProvider& operator=(TagProvider&&) = delete;

  virtual ~TagProvider() = default;

  [[nodiscard]] virtual bool Parse() const = 0;

 protected:
  std::string_view content_;
};

}  // namespace arboris

#endif  // SRC_DOM_TAG_PROVIDER_HPP_
