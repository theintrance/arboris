/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_ENCODING_ENCODING_HPP_
#define SRC_ENCODING_ENCODING_HPP_

#include <optional>
#include <string>
#include <string_view>

namespace arboris {

// The name the Encoding Standard gives a label ("UTF8", "latin1", "ks_c_5601-1987" become
// "utf-8", "windows-1252", "euc-kr"), or nullopt for a label it does not know.
std::optional<std::string> CanonicalEncoding(std::string_view label);

// What the bytes say about their own encoding: a byte order mark first, then a
// `<meta charset>` or `<meta http-equiv="Content-Type">` within the first 1024 bytes.
std::optional<std::string> SniffEncoding(std::string_view bytes);

// The bytes as UTF-8, or nullopt when nothing here converts from that encoding. UTF-8 and
// UTF-16 are converted in-house, everything else through iconv when the build has it.
std::optional<std::string> ToUtf8(std::string_view bytes, std::string_view encoding);

struct DecodedDocument {
  std::string utf8;

  // What the bytes were read as.
  std::string encoding;

  // What the caller or the bytes asked for, when that differs from `encoding` because
  // nothing here converts from it; the bytes were then read as UTF-8.
  std::optional<std::string> unsupported_encoding;
};

// Reads a document as UTF-8. A byte order mark decides the encoding; failing that, the
// caller's label; failing that, a <meta> in the first 1024 bytes; failing that, UTF-8.
DecodedDocument DecodeDocument(std::string_view bytes, std::string_view encoding_label = {});

}  // namespace arboris

#endif  // SRC_ENCODING_ENCODING_HPP_
