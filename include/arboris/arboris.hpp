/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef INCLUDE_ARBORIS_ARBORIS_HPP_
#define INCLUDE_ARBORIS_ARBORIS_HPP_

// The public API: everything a program needs to parse HTML and read the result.
//
//   DOMManager        parses one document and owns it (dom/dom_manager.hpp)
//   NodeRef           a handle to one element: tag, name, id, classes, attributes,
//                     text, outer/inner HTML, parent, children, siblings, ancestors
//                     (dom/node_ref.hpp)
//   DOMQuery          one result and the subtree the next query runs over (dom/dom_query.hpp)
//   QueryOptions      tag, name, class, attribute and text conditions (utils/query_options.hpp)
//   Selector          a parsed CSS selector list (selector/selector.hpp)
//   Tag, Attribute, TextRun
//
// DOMManager is neither copyable nor movable: it is the owner of every view the other
// types hand out. Keep it in place, or hold it through std::unique_ptr when it has to be
// returned from a function or stored in a container.
//
// The headers these include under dom/, string/, utils/ and encoding/ are implementation
// detail; their contents may change between versions.

#include "dom/dom_manager.hpp"
#include "dom/dom_query.hpp"
#include "dom/node_ref.hpp"
#include "selector/selector.hpp"
#include "utils/html_tokens.hpp"
#include "utils/query_options.hpp"
#include "utils/tag.hpp"

#endif  // INCLUDE_ARBORIS_ARBORIS_HPP_
