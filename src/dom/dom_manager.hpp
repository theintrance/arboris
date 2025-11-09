/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_DOM_DOM_MANAGER_HPP_
#define SRC_DOM_DOM_MANAGER_HPP_

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "dom/dom_builder.hpp"
#include "dom/dom_indexer.hpp"
#include "dom/html_token_parser.hpp"
#include "utils/string_pool.hpp"

namespace arboris {

class DOMManager {
 public:
  explicit DOMManager(std::string_view html_content);
  DOMManager(const DOMManager&) = delete;
  DOMManager& operator=(const DOMManager&) = delete;
  DOMManager(DOMManager&&) = delete;
  DOMManager& operator=(DOMManager&&) = delete;
  virtual ~DOMManager() = default;

  bool IsValid() const {
    ARBORIS_ASSERT(dom_builder_, "DOMBuilder is null");
    return dom_builder_->Validate();
  }

 private:
  void parse();

  std::unique_ptr<DOMBuilder> dom_builder_;
  std::unique_ptr<DOMIndexer> dom_indexer_;
  std::unique_ptr<HtmlTokenParser> html_token_parser_;
  std::shared_ptr<StringPool> string_pool_;
};

}  // namespace arboris

#endif  // SRC_DOM_DOM_MANAGER_HPP_
