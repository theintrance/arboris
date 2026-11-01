/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_MANAGER_HPP_
#define SRC_DOM_DOM_MANAGER_HPP_

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dom/dom_query.hpp"
#include "dom/dom_store.hpp"
#include "encoding/encoding.hpp"
#include "dom/node_ref.hpp"
#include "utils/assertion.hpp"
#include "utils/query_options.hpp"
#include "utils/string_pool.hpp"

namespace arboris {

// Parses one document and owns everything that came out of it.
//
// The HTML is copied once into the store, converted to UTF-8 on the way when it is in
// another encoding, and node ids, class names and attributes are views into that copy, so
// the caller's buffer does not have to outlive the DOM. Positions, parse_error_offset()
// among them, count bytes of that UTF-8 copy.
class DOMManager {
 public:
  // encoding_label names the bytes' encoding when the caller knows it, by any label the
  // Encoding Standard lists ("euc-kr", "latin1"). A byte order mark overrides it; without
  // either, a <meta charset> in the first 1024 bytes is used, and failing that UTF-8.
  explicit DOMManager(std::string_view html_content, std::string_view encoding_label = {});
  DOMManager(const DOMManager&) = delete;
  DOMManager& operator=(const DOMManager&) = delete;
  DOMManager(DOMManager&&) = delete;
  DOMManager& operator=(DOMManager&&) = delete;
  virtual ~DOMManager() = default;

  // The synthetic root (key 0, Tag::kDocument) above the top-level elements. Queries never
  // return it.
  [[nodiscard]] NodeRef GetRoot() const {
    ARBORIS_ASSERT(!store_.nodes().empty(), "Root node is nullptr.");
    return NodeRef(store_.nodes().front().key(), store_);
  }

  [[nodiscard]] std::optional<DOMQuery> Find(const QueryOptions& options) const;
  [[nodiscard]] std::optional<DOMQuery> Find(const std::string& id) const;
  [[nodiscard]] std::vector<DOMQuery> FindAll(const QueryOptions& options) const;

  // The byte offset where parsing stopped, or nullopt if the whole document was read. The
  // DOM holds everything before it.
  [[nodiscard]] std::optional<std::size_t> parse_error_offset() const noexcept {
    return parse_error_offset_;
  }

  // The encoding the bytes were read as, by its Encoding Standard name: "utf-8", "euc-kr".
  [[nodiscard]] std::string_view encoding() const noexcept {
    return input_.encoding;
  }

  // The encoding the caller or the page named when nothing here converts from it. The
  // bytes were then read as UTF-8, and text in them is likely garbled.
  [[nodiscard]] const std::optional<std::string>& unsupported_encoding() const noexcept {
    return input_.unsupported_encoding;
  }

 private:
  [[nodiscard]] DOMQuery rootQuery() const;

  DecodedDocument input_;
  DOMStore store_;
  std::shared_ptr<StringPool> string_pool_;
  std::optional<std::size_t> parse_error_offset_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_MANAGER_HPP_
