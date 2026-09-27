"""Feature areas for the html5lib-tests tokenizer cases.

A case is filed under the tokenizer feature it needs, worked out from its input and its
expected tokens. The file a case lives in is no guide: test1.test runs DOCTYPE, tags,
attributes and comments out of one array.

Passing a case has to mean the feature works, so a case whose expected output is its
input handed back unchanged is filed under Text no matter what it contains. `&Abreve`
without a semicolon must *not* be decoded, and filing it by the `&` in its input would
put 1979 such cases under Character references and show that area half finished while
nothing decodes at all.
"""

from __future__ import annotations

import re
from typing import TYPE_CHECKING, Callable, NamedTuple

if TYPE_CHECKING:
    from conftest import Case

# Display order, foundations first. Cases are matched in _MATCHERS order instead: the
# most specific token wins, so a DOCTYPE case is never mistaken for text.
MAJORS = ("Text", "Tags", "Attributes", "Character references", "Comments", "DOCTYPE")

# html5lib-tests token layouts, past the token name:
#   ["StartTag", name, attributes]   ["DOCTYPE", name, public id, system id, correct]
_ATTRIBUTES = 2
_PUBLIC_ID, _SYSTEM_ID = 2, 3
_ASCII_MAX = 0x7F

_REFERENCE = re.compile(r"&#?[0-9A-Za-z]+;?")


class Area(NamedTuple):
    major: str
    feature: str


def _expected_text(case: Case) -> str:
    """Every scrap of text the expected tokens carry, attribute values included."""
    out: list[str] = []
    for token in case.expected:
        if token[0] in ("Character", "Comment"):
            out.append(token[1])
        elif token[0] == "StartTag" and len(token) > _ATTRIBUTES:
            out.extend(token[_ATTRIBUTES].values())
    return "".join(out)


def _needs_decoding(case: Case) -> bool:
    """True when a reference in the input has to come out as something else.

    A reference that survives verbatim into the expected output belongs to a case about
    *not* decoding, which any tokenizer passes by leaving the text alone.
    """
    text = _expected_text(case)
    return any(reference not in text for reference in _REFERENCE.findall(case.input))


def _attributes(case: Case) -> dict[str, str]:
    for token in case.expected:
        if token[0] == "StartTag" and len(token) > _ATTRIBUTES and token[_ATTRIBUTES]:
            attributes: dict[str, str] = dict(token[_ATTRIBUTES])
            return attributes
    return {}


def _doctype(case: Case, kinds: set[str]) -> Area | None:
    if "DOCTYPE" not in kinds:
        return None
    token = next(t for t in case.expected if t[0] == "DOCTYPE")
    identified = len(token) > _SYSTEM_ID and (
        token[_PUBLIC_ID] is not None or token[_SYSTEM_ID] is not None
    )
    return Area("DOCTYPE", "public/system id" if identified else "name only")


def _comment(case: Case, kinds: set[str]) -> Area | None:
    if "Comment" not in kinds:
        return None
    if re.fullmatch(r"<!--.*-->", case.input, re.DOTALL):
        return Area("Comments", "well-formed")
    if case.input.startswith("<!--"):
        return Area("Comments", "unterminated")
    return Area("Comments", "bogus comment")


def _reference(case: Case, kinds: set[str]) -> Area | None:
    if not _needs_decoding(case):
        return None
    if re.search(r"&#[xX]", case.input):
        return Area("Character references", "numeric hex")
    if "&#" in case.input:
        return Area("Character references", "numeric decimal")
    if re.fullmatch(r"&[0-9A-Za-z]+;?", case.input):
        return Area("Character references", "named table")
    return Area("Character references", "named in context")


def _attribute(case: Case, kinds: set[str]) -> Area | None:
    if not _attributes(case):
        return None
    if "&" in case.input:
        return Area("Attributes", "reference in value")
    if re.search(r'=\s*"', case.input):
        return Area("Attributes", "double-quoted value")
    if re.search(r"=\s*'", case.input):
        return Area("Attributes", "single-quoted value")
    if re.search(r"=\s*[^\s\"'>]", case.input):
        return Area("Attributes", "unquoted value")
    return Area("Attributes", "valueless")


def _tag(case: Case, kinds: set[str]) -> Area | None:
    if not kinds & {"StartTag", "EndTag"}:
        return None
    if re.search(r"/\s*>", case.input):
        return Area("Tags", "self-closing")
    if kinds >= {"StartTag", "EndTag"}:
        return Area("Tags", "start + end")
    if "EndTag" in kinds:
        return Area("Tags", "end tag")
    return Area("Tags", "start tag")


def _text(case: Case, kinds: set[str]) -> Area:
    if "\r" in case.input:
        return Area("Text", "newline normalisation")
    if _REFERENCE.search(case.input):
        return Area("Text", "reference left as-is")
    if any(ord(character) > _ASCII_MAX for character in case.input):
        return Area("Text", "unicode")
    return Area("Text", "plain")


# Order is the precedence rule: the first matcher that recognises a case owns it.
_MATCHERS: tuple[Callable[[Case, set[str]], Area | None], ...] = (
    _doctype,
    _comment,
    _reference,
    _attribute,
    _tag,
)


def classify(case: Case) -> Area:
    """The one area a case belongs to."""
    kinds = {token[0] for token in case.expected}
    for matcher in _MATCHERS:
        area = matcher(case, kinds)
        if area is not None:
            return area
    return _text(case, kinds)
