/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef BENCHMARK_TOKENIZERS_HPP_
#define BENCHMARK_TOKENIZERS_HPP_

#include <cstddef>
#include <string_view>

namespace arboris::bench {

struct TokenizeResult {
  bool ok = false;
  std::size_t tokens = 0;
};

// Each tokenizer sets itself up for one document, tokenizes it and counts the tokens it
// hands out. Only the token count leaves the call, so no one pays for building output.
TokenizeResult TokenizeArboris(std::string_view input);

#ifdef ARBORIS_BENCH_LEXBOR
TokenizeResult TokenizeLexbor(std::string_view input);
#endif

}  // namespace arboris::bench

#endif  // BENCHMARK_TOKENIZERS_HPP_
