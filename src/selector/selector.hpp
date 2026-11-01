/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_SELECTOR_SELECTOR_HPP_
#define SRC_SELECTOR_SELECTOR_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dom/dom_types.hpp"
#include "dom/node_ref.hpp"

namespace arboris {

// One `[attr]`, `[attr=value]`, `[attr~=value]`, `[attr|=value]`, `[attr^=value]`,
// `[attr$=value]` or `[attr*=value]`, with the `i` flag making the value compare
// ASCII-case-insensitively.
struct AttributeSelector {
  enum class Match : std::uint8_t { kExists, kEquals, kIncludes, kDashMatch, kPrefix, kSuffix, kSubstring };

  std::string name;
  Match match = Match::kExists;
  std::string value;
  bool ignore_case = false;
};

struct CompoundSelector;

// `:first-child`, `:last-child`, `:only-child`, `:nth-child(an+b)`, `:nth-last-child(an+b)`,
// `:root`, `:empty` and `:not(compound)`.
struct PseudoClass {
  enum class Kind : std::uint8_t {
    kFirstChild, kLastChild, kOnlyChild, kNthChild, kNthLastChild, kRoot, kEmpty, kNot
  };

  Kind kind = Kind::kFirstChild;
  int a = 0;
  int b = 0;
  std::vector<CompoundSelector> argument;
};

// What one element has to be: `a.link[href]:first-child`. Everything is optional, so `*`
// is a compound with nothing set.
struct CompoundSelector {
  std::optional<std::string> tag;
  std::optional<std::string> id;
  std::vector<std::string> classes;
  std::vector<AttributeSelector> attributes;
  std::vector<PseudoClass> pseudo_classes;
};

enum class Combinator : std::uint8_t { kDescendant, kChild, kNextSibling, kSubsequentSibling };

// Compounds joined by combinators, kept right to left: compounds[0] is the element the
// selector is about, and combinators[i] relates compounds[i] to compounds[i + 1] on its left.
struct ComplexSelector {
  std::vector<CompoundSelector> compounds;
  std::vector<Combinator> combinators;
};

// A selector list, `div > a.link, #main p:first-child`, parsed once and matched many times.
//
// Type and attribute names are lower-cased, as HTML names are. CSS escapes, namespaces,
// `:nth-of-type`, `:is`/`:where`, and the state pseudo-classes are not read.
class Selector {
 public:
  // nullopt when the text is not a selector this reads.
  [[nodiscard]] static std::optional<Selector> Parse(std::string_view text);

  [[nodiscard]] bool Matches(const NodeRef& node) const;

  [[nodiscard]] const std::vector<ComplexSelector>& alternatives() const noexcept {
    return alternatives_;
  }

 private:
  std::vector<ComplexSelector> alternatives_;
};

// What a ComplexMatcher carries from one node to the next.
struct MatchState {
  // Per parent, the last sibling placed and its 1-based index, so siblings asked for in
  // document order are walked to from there.
  std::unordered_map<NodeKey, std::pair<NodeRef, int>> sibling_places;
  // Per parent, how many element children it has, for :nth-last-child.
  std::unordered_map<NodeKey, int> child_counts;
  // Whether compounds[index] and everything left of it held at a node, by (index, key),
  // so the descendant and sibling combinators never try the same pair twice.
  std::unordered_map<std::uint64_t, bool> outcomes;
  // For ~, by (index, parent key): the last node asked about and whether any sibling
  // before it matched compounds[index] leftwards, so the next node carries on from there.
  std::unordered_map<std::uint64_t, std::pair<NodeRef, bool>> preceding_matches;
};

// Matches one complex selector against any number of nodes of one document. Keep one
// for the whole query: what the walk over one node found serves the nodes after it.
class ComplexMatcher {
 public:
  explicit ComplexMatcher(const ComplexSelector& complex) noexcept : complex_(&complex) {}

  [[nodiscard]] bool Matches(const NodeRef& node);

 private:
  const ComplexSelector* complex_;
  MatchState state_;
};

// One-off matches, with nothing kept between calls.
[[nodiscard]] bool Matches(const CompoundSelector& compound, const NodeRef& node);
[[nodiscard]] bool Matches(const ComplexSelector& complex, const NodeRef& node);

}  // namespace arboris

#endif  // SRC_SELECTOR_SELECTOR_HPP_
