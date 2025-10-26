/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_TOKEN_PARSER_HPP_
#define SRC_DOM_TOKEN_PARSER_HPP_

#include <string_view>

namespace arboris {

class TokenParser {
 public:
  explicit TokenParser(std::string_view content) : content_(content) {}

  TokenParser(const TokenParser&) = delete;
  TokenParser& operator=(const TokenParser&) = delete;
  TokenParser(TokenParser&&) = delete;
  TokenParser& operator=(TokenParser&&) = delete;

  virtual ~TokenParser() = default;

  [[nodiscard]] virtual bool Parse() const = 0;

 protected:
  std::string_view content_;
};

}  // namespace arboris

#endif  // SRC_DOM_TOKEN_PARSER_HPP_
