"""html5lib-tests tokenizer cases against HtmlTokenParser (via html5lib_tokenizer_dump)."""

from __future__ import annotations

from conftest import Case, Dumper, normalize


def test_tokenizer(case: Case, dumper: Dumper) -> None:
    result = dumper.tokenize(case.input)
    assert result["ok"], f"Parse() returned false: {case.description}"
    assert normalize(result["tokens"]) == case.expected, case.description
