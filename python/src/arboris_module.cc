/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

// The Python module. A Document owns the DOMManager through a shared_ptr and every Node
// holds the same pointer, so a node outlives the document it came from in Python terms
// without any lifetime annotations.

#include <nanobind/nanobind.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/vector.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dom/dom_manager.hpp"
#include "dom/dom_query.hpp"
#include "dom/node_ref.hpp"
#include "selector/selector.hpp"
#include "utils/html_tokens.hpp"
#include "utils/query_options.hpp"
#include "utils/tag.hpp"

namespace nb = nanobind;

namespace {

using arboris::DOMManager;
using arboris::DOMQuery;
using arboris::NodeRef;

struct Document;

struct Node {
  std::shared_ptr<const DOMManager> dom;
  NodeRef ref;
};

struct Document {
  std::shared_ptr<const DOMManager> dom;
};

Node Wrap(const std::shared_ptr<const DOMManager>& dom, const NodeRef& ref) {
  return {dom, ref};
}

std::optional<Node> Wrap(const std::shared_ptr<const DOMManager>& dom, const std::optional<NodeRef>& ref) {
  if (!ref) {
    return std::nullopt;
  }
  return Node{dom, *ref};
}

std::optional<Node> Wrap(const std::shared_ptr<const DOMManager>& dom, const std::optional<DOMQuery>& query) {
  if (!query) {
    return std::nullopt;
  }
  return Node{dom, query->Get()};
}

std::vector<Node> Wrap(const std::shared_ptr<const DOMManager>& dom, const std::vector<DOMQuery>& queries) {
  std::vector<Node> out;
  out.reserve(queries.size());
  for (const auto& query : queries) {
    out.push_back({dom, query.Get()});
  }
  return out;
}

arboris::Selector ParseSelector(std::string_view text) {
  auto selector = arboris::Selector::Parse(text);
  if (!selector) {
    throw nb::value_error(("not a selector: " + std::string(text)).c_str());
  }
  return std::move(*selector);
}

// Builds the C++ query from the keyword arguments of find()/find_all().
arboris::QueryOptions Options(const std::optional<std::string>& tag, nb::object classes, nb::object attrs,
                              nb::object text) {
  arboris::QueryOptions options;
  if (tag) {
    options.name = *tag;
  }
  if (!classes.is_none()) {
    arboris::ClassSet set;
    if (nb::isinstance<nb::str>(classes)) {
      set.insert(nb::cast<std::string>(classes));
    } else {
      for (nb::handle item : classes) {
        set.insert(nb::cast<std::string>(item));
      }
    }
    options.classes = std::move(set);
  }
  if (!attrs.is_none()) {
    arboris::AttributeMap map;
    for (const auto& [key, value] : nb::cast<nb::dict>(attrs)) {
      auto& values = map[nb::cast<std::string>(key)];
      if (!value.is_none()) {
        values.insert(nb::cast<std::string>(value));
      }
    }
    options.attributes = std::move(map);
  }
  if (!text.is_none()) {
    if (nb::isinstance<nb::str>(text)) {
      options.text.emplace(nb::cast<std::string>(text));
    } else {
      nb::object predicate = nb::borrow(text);
      options.text.emplace([predicate](std::string_view value) { return nb::cast<bool>(predicate(value)); });
    }
  }
  return options;
}

nb::dict Attrs(const NodeRef& ref) {
  nb::dict out;
  for (const auto& attribute : ref.attributes()) {
    out[nb::str(attribute.name.data(), attribute.name.size())] =
        nb::str(attribute.value.data(), attribute.value.size());
  }
  return out;
}

std::vector<std::string> Classes(const NodeRef& ref) {
  std::vector<std::string> out;
  for (const auto class_name : ref.classes()) {
    out.emplace_back(class_name);
  }
  return out;
}

std::string Repr(const Node& node) {
  std::string out = "<arboris.Node ";
  if (node.ref.tag() == arboris::Tag::kDocument) {
    out += "#document";
  } else {
    out += std::string(node.ref.name());
    if (!node.ref.id().empty()) {
      out += " id=\"" + std::string(node.ref.id()) + "\"";
    }
  }
  return out + ">";
}

// find/find_all/select/select_one are the same on a document and on a node: the node form
// searches its descendants only.
template <typename Class>
void DefineQueries(Class& cls, DOMQuery (*query_of)(const typename Class::Type&)) {
  using Self = typename Class::Type;
  cls.def(
         "select",
         [query_of](const Self& self, std::string_view selector) {
           return Wrap(self.dom, query_of(self).SelectAll(ParseSelector(selector)));
         },
         nb::arg("selector"), "Every element the CSS selector matches, in document order.")
      .def(
          "select_one",
          [query_of](const Self& self, std::string_view selector) {
            return Wrap(self.dom, query_of(self).Select(ParseSelector(selector)));
          },
          nb::arg("selector"), "The first element the CSS selector matches, or None.")
      .def(
          "find_all",
          [query_of](const Self& self, const std::optional<std::string>& tag, nb::object classes, nb::object attrs,
                     nb::object text) {
            return Wrap(self.dom, query_of(self).FindAll(Options(tag, classes, attrs, text)));
          },
          nb::arg("tag") = nb::none(), nb::arg("classes") = nb::none(), nb::arg("attrs") = nb::none(),
          nb::arg("text") = nb::none(),
          "Every element matching all of the conditions: tag name, class name(s), attributes "
          "(name -> value, or None for any value) and text (a string, or a predicate on it).")
      .def(
          "find",
          [query_of](const Self& self, const std::optional<std::string>& tag, nb::object classes, nb::object attrs,
                     nb::object text) {
            return Wrap(self.dom, query_of(self).Find(Options(tag, classes, attrs, text)));
          },
          nb::arg("tag") = nb::none(), nb::arg("classes") = nb::none(), nb::arg("attrs") = nb::none(),
          nb::arg("text") = nb::none(), "The first element matching all of the conditions, or None.")
      .def(
          "get_by_id",
          [query_of](const Self& self, const std::string& id) { return Wrap(self.dom, query_of(self).Find(id)); },
          nb::arg("id"), "The first element with the id, or None.");
}

DOMQuery QueryOf(const Document& document) {
  return document.dom->Query(document.dom->GetRoot());
}

DOMQuery QueryOf(const Node& node) {
  return node.dom->Query(node.ref);
}

Document Parse(nb::object data, const std::optional<std::string>& encoding) {
  if (nb::isinstance<nb::bytes>(data)) {
    const auto bytes = nb::cast<nb::bytes>(data);
    return {std::make_shared<DOMManager>(std::string_view(bytes.c_str(), bytes.size()), encoding.value_or(""))};
  }
  if (nb::isinstance<nb::str>(data)) {
    // A str is already text, so the bytes are UTF-8 whatever a <meta> in it says.
    const auto text = nb::cast<std::string_view>(data);
    return {std::make_shared<DOMManager>(text, "utf-8")};
  }
  throw nb::type_error("parse() takes str or bytes");
}

}  // namespace

NB_MODULE(_arboris, m) {
  m.doc() = "arboris: an HTML parser with a queryable DOM.";
  m.attr("__version__") = ARBORIS_VERSION;

  nb::class_<Node> node(m, "Node", "One element of a parsed document.");
  node.def_prop_ro("tag", [](const Node& n) { return n.ref.name(); }, "The tag name, lower-case: \"div\", \"my-card\".")
      .def_prop_ro("id", [](const Node& n) { return n.ref.id(); })
      .def_prop_ro("classes", [](const Node& n) { return Classes(n.ref); })
      .def_prop_ro("attrs", [](const Node& n) { return Attrs(n.ref); },
                   "The attributes, name -> value. An attribute written without a value maps to \"\".")
      .def_prop_ro("text", [](const Node& n) { return n.ref.text_content(); },
                   "The text of this element and everything inside it, references decoded.")
      .def_prop_ro("outer_html", [](const Node& n) { return n.ref.outer_html(); },
                   "The element as written in the source, start tag to end tag.")
      .def_prop_ro("inner_html", [](const Node& n) { return n.ref.inner_html(); },
                   "What is written between the start tag and the end tag.")
      .def_prop_ro(
          "parent",
          [](const Node& n) -> std::optional<Node> {
            const NodeRef parent = n.ref.parent();
            return parent.key() == n.ref.key() ? std::nullopt : std::optional(Wrap(n.dom, parent));
          },
          "The parent element, or None for the document root.")
      .def_prop_ro("children",
                   [](const Node& n) {
                     std::vector<Node> out;
                     for (const NodeRef child : n.ref.children()) {
                       out.push_back(Wrap(n.dom, child));
                     }
                     return out;
                   })
      .def_prop_ro("ancestors",
                   [](const Node& n) {
                     std::vector<Node> out;
                     for (const NodeRef ancestor : n.ref.ancestors()) {
                       out.push_back(Wrap(n.dom, ancestor));
                     }
                     return out;
                   },
                   "The parent, its parent and so on, up to but not including the document root.")
      .def_prop_ro("next_sibling", [](const Node& n) { return Wrap(n.dom, n.ref.next_sibling()); })
      .def_prop_ro("previous_sibling", [](const Node& n) { return Wrap(n.dom, n.ref.previous_sibling()); })
      // is_operator: comparing with something that is not a Node gives NotImplemented, so
      // `node != None` is True instead of a TypeError.
      .def(
          "__eq__", [](const Node& a, const Node& b) { return a.dom == b.dom && a.ref.key() == b.ref.key(); },
          nb::is_operator())
      .def("__hash__", [](const Node& n) { return static_cast<std::size_t>(n.ref.key()); })
      .def("__repr__", &Repr);
  DefineQueries(node, &QueryOf);

  nb::class_<Document> document(m, "Document", "A parsed HTML document; see parse().");
  document.def_prop_ro("root", [](const Document& d) { return Wrap(d.dom, d.dom->GetRoot()); },
                       "The document root above the top-level elements. Queries never return it.")
      .def_prop_ro("html", [](const Document& d) { return d.dom->GetRoot().outer_html(); },
                   "The whole document as parsed, in UTF-8.")
      .def_prop_ro("encoding", [](const Document& d) { return d.dom->encoding(); },
                   "The encoding the bytes were read as, by its Encoding Standard name.")
      .def_prop_ro("unsupported_encoding", [](const Document& d) { return d.dom->unsupported_encoding(); },
                   "The encoding the input named when it could not be converted, else None.")
      .def_prop_ro("parse_error_offset", [](const Document& d) { return d.dom->parse_error_offset(); },
                   "The byte offset where parsing stopped, or None if the whole input was read.")
      .def("__repr__", [](const Document& d) {
        return "<arboris.Document " + std::to_string(d.dom->GetRoot().outer_html().size()) + " bytes>";
      });
  DefineQueries(document, &QueryOf);

  m.def("parse", &Parse, nb::arg("data"), nb::arg("encoding") = nb::none(),
        "Parses HTML. bytes are decoded by their byte order mark, the encoding argument, or a "
        "<meta charset>, in that order, else as UTF-8; a str is text already.");
}
