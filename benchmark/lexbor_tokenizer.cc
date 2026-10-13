/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include <lexbor/html/tokenizer.h>

#include <string_view>

#include "benchmark/tokenizers.hpp"

namespace arboris::bench {
namespace {

lxb_html_token_t* CountToken(lxb_html_tokenizer_t*, lxb_html_token_t* token, void* ctx) {
  if (token->tag_id != LXB_TAG__END_OF_FILE) {
    ++static_cast<TokenizeResult*>(ctx)->tokens;
  }
  return token;
}

}  // anonymous namespace

TokenizeResult TokenizeLexbor(std::string_view input) {
  TokenizeResult result;
  lxb_html_tokenizer_t* tokenizer = lxb_html_tokenizer_create();
  if (lxb_html_tokenizer_init(tokenizer) != LXB_STATUS_OK) {
    lxb_html_tokenizer_destroy(tokenizer);
    return result;
  }
  lxb_html_tokenizer_callback_token_done_set(tokenizer, CountToken, &result);

  const auto* data = reinterpret_cast<const lxb_char_t*>(input.data());
  result.ok = lxb_html_tokenizer_begin(tokenizer) == LXB_STATUS_OK &&
              lxb_html_tokenizer_chunk(tokenizer, data, input.size()) == LXB_STATUS_OK &&
              lxb_html_tokenizer_end(tokenizer) == LXB_STATUS_OK;
  lxb_html_tokenizer_destroy(tokenizer);
  return result;
}

}  // namespace arboris::bench
