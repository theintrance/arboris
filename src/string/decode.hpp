/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_STRING_DECODE_HPP_
#define SRC_STRING_DECODE_HPP_

#include <cstddef>
#include <string>
#include <string_view>

namespace arboris {

// How a run of source bytes becomes the text a reader sees.
enum class TextMode {
  // <script> and <style>: newlines are normalised, nothing else changes.
  kRawText,
  // Text and RCDATA (<textarea>, <title>): character references are decoded too.
  kText,
  // As kText, except that a reference without ';' followed by '=' or a letter or digit is
  // left alone, as the HTML tokenizer leaves `?a=1&copy=2` alone.
  kAttributeValue,
};

// Decoded bytes per source byte, at most. A buffer reserved at this ratio never grows.
std::size_t DecodedSizeBound(std::size_t source_size);

// True when decoding would give back something other than the input itself.
bool NeedsDecoding(std::string_view in, TextMode mode);

// Writes in to out with "\r\n" and "\r" turned into "\n" and, by mode, character
// references decoded. out has room for DecodedSizeBound(in.size()) bytes; the length
// written is returned.
std::size_t DecodeInto(std::string_view in, TextMode mode, char* out);

void AppendDecoded(std::string_view in, TextMode mode, std::string* out);

}  // namespace arboris

#endif  // SRC_STRING_DECODE_HPP_
