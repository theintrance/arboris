/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#include "selector/selector.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "string/string.hpp"
#include "utils/tag.hpp"

namespace arboris {

namespace {

bool IsNameStart(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || static_cast<unsigned char>(c) >= 0x80;
}

bool IsNameChar(char c) {
  return IsNameStart(c) || (c >= '0' && c <= '9') || c == '-';
}

// Reads the selector text left to right. Every read* returns false on text it does not
// understand and leaves the whole parse failed.
class Parser {
 public:
  explicit Parser(std::string_view text) : text_(text) {}

  bool ReadList(std::vector<ComplexSelector>* out) {
    do {
      skipSpace();
      ComplexSelector complex;
      if (!readComplex(&complex)) {
        return false;
      }
      out->push_back(std::move(complex));
      skipSpace();
    } while (take(','));
    return atEnd();
  }

 private:
  [[nodiscard]] bool atEnd() const {
    return pos_ >= text_.size();
  }

  [[nodiscard]] char peek() const {
    return atEnd() ? '\0' : text_[pos_];
  }

  bool take(char c) {
    if (peek() != c) {
      return false;
    }
    ++pos_;
    return true;
  }

  void skipSpace() {
    pos_ = SkipHtmlWhitespace(text_, pos_);
  }

  std::optional<std::string> readIdent() {
    std::size_t end = pos_;
    if (end < text_.size() && text_[end] == '-') {
      ++end;
    }
    if (end >= text_.size() || !IsNameStart(text_[end])) {
      return std::nullopt;
    }
    while (end < text_.size() && IsNameChar(text_[end])) {
      ++end;
    }
    std::string out(text_.substr(pos_, end - pos_));
    pos_ = end;
    return out;
  }

  std::optional<std::string> readString() {
    const char quote = peek();
    if (quote != '"' && quote != '\'') {
      return readIdent();
    }
    const std::size_t end = text_.find(quote, pos_ + 1);
    if (end == std::string_view::npos) {
      return std::nullopt;
    }
    std::string out(text_.substr(pos_ + 1, end - pos_ - 1));
    pos_ = end + 1;
    return out;
  }

  // Reads compounds and the combinators between them, then turns the list around so the
  // subject comes first.
  bool readComplex(ComplexSelector* out) {
    std::vector<CompoundSelector> compounds;
    std::vector<Combinator> combinators;

    CompoundSelector first;
    if (!readCompound(&first)) {
      return false;
    }
    compounds.push_back(std::move(first));

    for (;;) {
      const std::size_t before = pos_;
      skipSpace();
      Combinator combinator = Combinator::kDescendant;
      if (take('>')) {
        combinator = Combinator::kChild;
      } else if (take('+')) {
        combinator = Combinator::kNextSibling;
      } else if (take('~')) {
        combinator = Combinator::kSubsequentSibling;
      } else if (pos_ == before || atEnd() || peek() == ',') {
        pos_ = before;
        break;
      }
      skipSpace();

      CompoundSelector next;
      if (!readCompound(&next)) {
        return false;
      }
      compounds.push_back(std::move(next));
      combinators.push_back(combinator);
    }

    std::ranges::reverse(compounds);
    std::ranges::reverse(combinators);
    out->compounds = std::move(compounds);
    out->combinators = std::move(combinators);
    return true;
  }

  bool readCompound(CompoundSelector* out) {
    bool any = false;
    if (take('*')) {
      any = true;
    } else if (IsNameStart(peek()) || peek() == '-') {
      auto tag = readIdent();
      if (!tag) {
        return false;
      }
      out->tag = AsciiLowerCase(*tag);
      any = true;
    }

    for (;;) {
      if (take('#')) {
        auto id = readIdent();
        if (!id) {
          return false;
        }
        out->id = std::move(*id);
      } else if (take('.')) {
        auto class_name = readIdent();
        if (!class_name) {
          return false;
        }
        out->classes.push_back(std::move(*class_name));
      } else if (take('[')) {
        AttributeSelector attribute;
        if (!readAttribute(&attribute)) {
          return false;
        }
        out->attributes.push_back(std::move(attribute));
      } else if (take(':')) {
        PseudoClass pseudo;
        if (!readPseudoClass(&pseudo)) {
          return false;
        }
        out->pseudo_classes.push_back(std::move(pseudo));
      } else {
        return any;
      }
      any = true;
    }
  }

  bool readAttribute(AttributeSelector* out) {
    skipSpace();
    auto name = readIdent();
    if (!name) {
      return false;
    }
    out->name = AsciiLowerCase(*name);
    skipSpace();

    if (take(']')) {
      return true;
    }
    switch (peek()) {
      case '=': out->match = AttributeSelector::Match::kEquals; break;
      case '~': out->match = AttributeSelector::Match::kIncludes; break;
      case '|': out->match = AttributeSelector::Match::kDashMatch; break;
      case '^': out->match = AttributeSelector::Match::kPrefix; break;
      case '$': out->match = AttributeSelector::Match::kSuffix; break;
      case '*': out->match = AttributeSelector::Match::kSubstring; break;
      default: return false;
    }
    ++pos_;
    if (out->match != AttributeSelector::Match::kEquals && !take('=')) {
      return false;
    }

    skipSpace();
    auto value = readString();
    if (!value) {
      return false;
    }
    out->value = std::move(*value);
    skipSpace();
    if (take('i') || take('I')) {
      out->ignore_case = true;
      skipSpace();
    } else if (take('s') || take('S')) {
      skipSpace();
    }
    return take(']');
  }

  bool readPseudoClass(PseudoClass* out) {
    auto name = readIdent();
    if (!name) {
      return false;
    }
    const std::string lowered = AsciiLowerCase(*name);

    if (lowered == "first-child") {
      out->kind = PseudoClass::Kind::kFirstChild;
    } else if (lowered == "last-child") {
      out->kind = PseudoClass::Kind::kLastChild;
    } else if (lowered == "only-child") {
      out->kind = PseudoClass::Kind::kOnlyChild;
    } else if (lowered == "root") {
      out->kind = PseudoClass::Kind::kRoot;
    } else if (lowered == "empty") {
      out->kind = PseudoClass::Kind::kEmpty;
    } else if (lowered == "nth-child" || lowered == "nth-last-child") {
      out->kind = lowered == "nth-child" ? PseudoClass::Kind::kNthChild : PseudoClass::Kind::kNthLastChild;
      return take('(') && readNth(&out->a, &out->b) && take(')');
    } else if (lowered == "not") {
      out->kind = PseudoClass::Kind::kNot;
      if (!take('(')) {
        return false;
      }
      skipSpace();
      CompoundSelector argument;
      if (!readCompound(&argument)) {
        return false;
      }
      out->argument.push_back(std::move(argument));
      skipSpace();
      return take(')');
    } else {
      return false;
    }
    return true;
  }

  // `an+b`, `odd`, `even`, or a lone integer, with spaces allowed around the sign.
  bool readNth(int* a, int* b) {
    skipSpace();
    const std::size_t end = text_.find(')', pos_);
    if (end == std::string_view::npos) {
      return false;
    }
    std::string spec = AsciiLowerCase(text_.substr(pos_, end - pos_));
    std::erase_if(spec, [](char c) { return kHtmlWhitespaceSet.Contains(c); });
    pos_ = end;

    if (spec == "odd") {
      *a = 2;
      *b = 1;
      return true;
    }
    if (spec == "even") {
      *a = 2;
      *b = 0;
      return true;
    }

    const std::size_t n = spec.find('n');
    const std::string_view a_text = n == std::string::npos ? "" : std::string_view(spec).substr(0, n);
    const std::string_view b_text = n == std::string::npos ? spec : std::string_view(spec).substr(n + 1);

    if (n == std::string::npos) {
      *a = 0;
    } else if (a_text.empty() || a_text == "+") {
      *a = 1;
    } else if (a_text == "-") {
      *a = -1;
    } else if (!readInt(a_text, a)) {
      return false;
    }

    if (b_text.empty()) {
      *b = 0;
      return n != std::string::npos;
    }
    return (n == std::string::npos || b_text[0] == '+' || b_text[0] == '-') && readInt(b_text, b);
  }

  static bool readInt(std::string_view text, int* out) {
    bool negative = false;
    if (!text.empty() && (text[0] == '+' || text[0] == '-')) {
      negative = text[0] == '-';
      text.remove_prefix(1);
    }
    if (text.empty() || text.size() > 9) {
      return false;
    }
    int value = 0;
    for (const char c : text) {
      if (c < '0' || c > '9') {
        return false;
      }
      value = value * 10 + (c - '0');
    }
    *out = negative ? -value : value;
    return true;
  }

  std::string_view text_;
  std::size_t pos_ = 0;
};

bool IsRoot(const NodeRef& node) {
  return node.tag() == Tag::kDocument;
}

// The document element: the first element under the synthetic root. A page can carry
// markup after </html>, and that is not the root.
bool IsDocumentElement(const NodeRef& node) {
  const NodeRef parent = node.parent();
  return IsRoot(parent) && parent.key() + 1 == node.key();
}

bool MatchesAttribute(const AttributeSelector& selector, const NodeRef& node) {
  const auto attributes = node.attributes();
  const auto found = std::ranges::find(attributes, selector.name, &Attribute::name);
  if (found == attributes.end()) {
    return false;
  }
  if (selector.match == AttributeSelector::Match::kExists) {
    return true;
  }

  const std::string_view value = found->value;
  const std::string_view wanted = selector.value;
  const auto equals = [&](std::string_view lhs, std::string_view rhs) {
    return selector.ignore_case ? EqualsIgnoringCase(lhs, rhs) : lhs == rhs;
  };

  switch (selector.match) {
    case AttributeSelector::Match::kEquals:
      return equals(value, wanted);
    case AttributeSelector::Match::kIncludes: {
      if (wanted.empty() || FindNextOf(wanted, 0, kHtmlWhitespaceSet) != std::string_view::npos) {
        return false;
      }
      for (std::size_t pos = SkipHtmlWhitespace(value, 0); pos < value.size();) {
        const std::size_t end = std::min(FindNextOf(value, pos, kHtmlWhitespaceSet), value.size());
        if (equals(value.substr(pos, end - pos), wanted)) {
          return true;
        }
        pos = SkipHtmlWhitespace(value, end);
      }
      return false;
    }
    case AttributeSelector::Match::kDashMatch:
      return equals(value, wanted) || (value.size() > wanted.size() && value[wanted.size()] == '-' &&
                                       equals(value.substr(0, wanted.size()), wanted));
    case AttributeSelector::Match::kPrefix:
      return !wanted.empty() && value.size() >= wanted.size() && equals(value.substr(0, wanted.size()), wanted);
    case AttributeSelector::Match::kSuffix:
      return !wanted.empty() && value.size() >= wanted.size() &&
             equals(value.substr(value.size() - wanted.size()), wanted);
    case AttributeSelector::Match::kSubstring: {
      if (wanted.empty() || value.size() < wanted.size()) {
        return false;
      }
      return (selector.ignore_case ? FindIgnoringCase(value, wanted, 0) : value.find(wanted)) !=
             std::string_view::npos;
    }
    case AttributeSelector::Match::kExists:
      return true;
  }
  return false;
}

int ChildCount(const NodeRef& parent, MatchState* state) {
  if (state) {
    if (const auto found = state->child_counts.find(parent.key()); found != state->child_counts.end()) {
      return found->second;
    }
  }
  int count = 0;
  for (auto child = parent.first_child(); child; child = child->next_sibling()) {
    ++count;
  }
  if (state) {
    state->child_counts.emplace(parent.key(), count);
  }
  return count;
}

struct SiblingPlace {
  // 1-based, among the element siblings.
  int index;
  std::optional<NodeRef> previous;
};

// Where the node stands among its element siblings. With state, nodes asked for in
// document order are walked to from the last one under the same parent rather than from
// the first child again.
SiblingPlace PlaceAmongSiblings(const NodeRef& node, MatchState* state) {
  const NodeRef parent = node.parent();
  std::optional<NodeRef> sibling = parent.first_child();
  int index = 1;
  if (state) {
    const auto found = state->sibling_places.find(parent.key());
    if (found != state->sibling_places.end() && found->second.first.key() <= node.key()) {
      sibling = found->second.first;
      index = found->second.second;
    }
  }
  std::optional<NodeRef> previous;
  for (; sibling && sibling->key() < node.key(); sibling = sibling->next_sibling()) {
    previous = sibling;
    ++index;
  }
  if (state) {
    state->sibling_places.insert_or_assign(parent.key(), std::pair{node, index});
  }
  return {index, previous};
}

bool MatchesNth(int a, int b, int index) {
  if (a == 0) {
    return index == b;
  }
  return (index - b) % a == 0 && (index - b) / a >= 0;
}

bool MatchesCompound(const CompoundSelector& compound, const NodeRef& node, MatchState* state);

bool MatchesPseudoClass(const PseudoClass& pseudo, const NodeRef& node, MatchState* state) {
  switch (pseudo.kind) {
    case PseudoClass::Kind::kFirstChild:
      return node.is_first_child();
    case PseudoClass::Kind::kLastChild:
      return !node.next_sibling().has_value();
    case PseudoClass::Kind::kOnlyChild:
      return node.is_first_child() && !node.next_sibling().has_value();
    case PseudoClass::Kind::kNthChild:
      return MatchesNth(pseudo.a, pseudo.b, PlaceAmongSiblings(node, state).index);
    case PseudoClass::Kind::kNthLastChild:
      return MatchesNth(pseudo.a, pseudo.b,
                        ChildCount(node.parent(), state) - PlaceAmongSiblings(node, state).index + 1);
    case PseudoClass::Kind::kRoot:
      return IsDocumentElement(node);
    case PseudoClass::Kind::kEmpty:
      return node.sub_tree_size() == 1 && node.text_runs().empty();
    case PseudoClass::Kind::kNot:
      return std::ranges::none_of(pseudo.argument,
                                  [&](const CompoundSelector& c) { return MatchesCompound(c, node, state); });
  }
  return false;
}

bool MatchesCompound(const CompoundSelector& compound, const NodeRef& node, MatchState* state) {
  if (IsRoot(node)) {
    return false;
  }
  if (compound.tag && node.name() != *compound.tag) {
    return false;
  }
  if (compound.id && node.id() != *compound.id) {
    return false;
  }
  const auto classes = node.classes();
  for (const auto& class_name : compound.classes) {
    if (std::ranges::find(classes, class_name) == classes.end()) {
      return false;
    }
  }
  for (const auto& attribute : compound.attributes) {
    if (!MatchesAttribute(attribute, node)) {
      return false;
    }
  }
  for (const auto& pseudo : compound.pseudo_classes) {
    if (!MatchesPseudoClass(pseudo, node, state)) {
      return false;
    }
  }
  return true;
}

bool MatchesFrom(const ComplexSelector& complex, std::size_t index, const NodeRef& node, MatchState* state);

std::uint64_t MemoKey(std::size_t index, NodeKey key) {
  return (static_cast<std::uint64_t>(index) << 32) | key;
}

// Whether compounds[index] and everything left of it hold at some element sibling before
// node. With state, nodes asked for in document order continue the walk from the last one.
bool AnyPrecedingSiblingMatches(const ComplexSelector& complex, std::size_t index, const NodeRef& node,
                                MatchState* state) {
  const NodeRef parent = node.parent();
  std::optional<NodeRef> sibling = parent.first_child();
  bool any = false;
  const std::uint64_t memo_key = MemoKey(index, parent.key());
  if (state) {
    const auto found = state->preceding_matches.find(memo_key);
    if (found != state->preceding_matches.end() && found->second.first.key() <= node.key()) {
      sibling = found->second.first;
      any = found->second.second;
    }
  }
  for (; sibling && sibling->key() < node.key(); sibling = sibling->next_sibling()) {
    any = any || MatchesFrom(complex, index, *sibling, state);
  }
  if (state) {
    state->preceding_matches.insert_or_assign(memo_key, std::pair{node, any});
  }
  return any;
}

// Matches compounds[index] and everything to its left against node, by walking the
// combinator from node. Backtracking alone is exponential in the number of compounds, so
// MatchesFrom remembers each (index, node) outcome.
bool MatchesFromHere(const ComplexSelector& complex, std::size_t index, const NodeRef& node, MatchState* state) {
  if (!MatchesCompound(complex.compounds[index], node, state)) {
    return false;
  }
  if (index + 1 == complex.compounds.size()) {
    return true;
  }

  switch (complex.combinators[index]) {
    case Combinator::kChild: {
      const NodeRef parent = node.parent();
      return !IsRoot(parent) && MatchesFrom(complex, index + 1, parent, state);
    }
    case Combinator::kDescendant:
      for (NodeRef ancestor = node.parent(); !IsRoot(ancestor); ancestor = ancestor.parent()) {
        if (MatchesFrom(complex, index + 1, ancestor, state)) {
          return true;
        }
      }
      return false;
    case Combinator::kNextSibling: {
      const auto previous = PlaceAmongSiblings(node, state).previous;
      return previous && MatchesFrom(complex, index + 1, *previous, state);
    }
    case Combinator::kSubsequentSibling:
      return AnyPrecedingSiblingMatches(complex, index + 1, node, state);
  }
  return false;
}

bool MatchesFrom(const ComplexSelector& complex, std::size_t index, const NodeRef& node, MatchState* state) {
  if (!state || index == 0) {
    return MatchesFromHere(complex, index, node, state);
  }
  const std::uint64_t memo_key = MemoKey(index, node.key());
  if (const auto found = state->outcomes.find(memo_key); found != state->outcomes.end()) {
    return found->second;
  }
  const bool matched = MatchesFromHere(complex, index, node, state);
  state->outcomes.emplace(memo_key, matched);
  return matched;
}

}  // anonymous namespace

bool Matches(const CompoundSelector& compound, const NodeRef& node) {
  return MatchesCompound(compound, node, nullptr);
}

bool Matches(const ComplexSelector& complex, const NodeRef& node) {
  return ComplexMatcher(complex).Matches(node);
}

bool ComplexMatcher::Matches(const NodeRef& node) {
  return !complex_->compounds.empty() && MatchesFrom(*complex_, 0, node, &state_);
}

std::optional<Selector> Selector::Parse(std::string_view text) {
  Selector selector;
  Parser parser(text);
  if (!parser.ReadList(&selector.alternatives_)) {
    return std::nullopt;
  }
  return selector;
}

bool Selector::Matches(const NodeRef& node) const {
  return std::ranges::any_of(alternatives_, [&](const ComplexSelector& c) { return arboris::Matches(c, node); });
}

}  // namespace arboris
