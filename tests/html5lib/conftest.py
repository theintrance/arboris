"""pytest plumbing for the html5lib-tests tokenizer suite.

Cases are identified as "<file>:<index>" (e.g. "test1.test:12"), where <index> is the
position in the file's "tests" array at the pinned html5lib-tests commit.

Every well-formed Data-state case is a goal. Cases html5lib-tests marks with parse
"errors" are broken HTML, which arboris does not aim to support, so they are skipped
unless --include-broken is given. Individual cases can be dropped in
tokenizer_excludes.txt.

The dump binary and test data are looked up in the CMake build directory ($ARBORIS_BUILD_DIR,
default "build/"), configured with -DARBORIS_HTML5LIB_TESTS=ON:

  uv run --group test pytest tests/html5lib [--all] [-k "test1.test:12]"]
"""

from __future__ import annotations

import json
import os
import re
import subprocess
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import pytest

DATA_STATE = "Data state"
DEFAULT_EXCLUDES = Path(__file__).with_name("tokenizer_excludes.txt")
BUILD_DIR = Path(os.environ.get("ARBORIS_BUILD_DIR", Path(__file__).parents[2] / "build"))
DEFAULT_DUMP = BUILD_DIR / "tests" / "html5lib_tokenizer_dump"
DEFAULT_TESTS_DIR = BUILD_DIR / "_deps" / "html5lib_tests-src" / "tokenizer"
_ESCAPE_RE = re.compile(r"\\u([0-9A-Fa-f]{4})")


@dataclass
class Case:
    case_id: str
    description: str
    input: str
    expected: list[Any]


def unescape(value: Any) -> Any:
    """Decode the extra \\uXXXX layer of "doubleEscaped" tests."""
    if isinstance(value, str):
        return _ESCAPE_RE.sub(lambda m: chr(int(m.group(1), 16)), value)
    if isinstance(value, list):
        return [unescape(v) for v in value]
    if isinstance(value, dict):
        return {unescape(k): unescape(v) for k, v in value.items()}
    return value


def normalize(tokens: list[Any]) -> list[Any]:
    """Merge adjacent Character tokens, as html5lib-tests expects."""
    result: list[Any] = []
    for token in tokens:
        if token[0] == "Character" and result and result[-1][0] == "Character":
            result[-1] = ["Character", result[-1][1] + token[1]]
        else:
            result.append(list(token))
    return result


def load_cases(tests_dir: Path, include_broken: bool = False) -> list[Case]:
    """Load the cases that run in the Data state and are in scope.

    Other initial states are skipped because HtmlTokenParser has no state switching.
    Cases with a parse "errors" entry are broken HTML and are skipped as out of scope.
    """
    cases: list[Case] = []
    for path in sorted(tests_dir.glob("*.test")):
        tests = json.loads(path.read_text(encoding="utf-8")).get("tests", [])
        for index, test in enumerate(tests):
            if DATA_STATE not in test.get("initialStates", [DATA_STATE]):
                continue
            if test.get("errors") and not include_broken:
                continue
            data = test
            if test.get("doubleEscaped"):
                data = {"input": unescape(test["input"]), "output": unescape(test["output"])}
            cases.append(
                Case(
                    case_id=f"{path.name}:{index}",
                    description=test.get("description", ""),
                    input=data["input"],
                    expected=normalize(data["output"]),
                )
            )
    return cases


def load_excludes(path: Path) -> set[str]:
    if not path.is_file():
        return set()
    excludes = set()
    for line in path.read_text(encoding="utf-8").splitlines():
        case_id = line.split("#", 1)[0].strip()
        if case_id:
            excludes.add(case_id)
    return excludes


class Dumper:
    """Keeps one tokenizer_dump process alive and feeds it length-prefixed inputs."""

    def __init__(self, binary: Path) -> None:
        self._proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def tokenize(self, text: str) -> dict[str, Any]:
        assert self._proc.stdin and self._proc.stdout
        data = text.encode("utf-8", "surrogatepass")
        self._proc.stdin.write(f"{len(data)}\n".encode() + data)
        self._proc.stdin.flush()
        line = self._proc.stdout.readline()
        if not line:
            raise RuntimeError(f"tokenizer_dump exited (code {self._proc.poll()})")
        result: dict[str, Any] = json.loads(line.decode("utf-8", "surrogatepass"))
        return result

    def close(self) -> None:
        if self._proc.stdin:
            self._proc.stdin.close()
        self._proc.wait()


def pytest_addoption(parser: pytest.Parser) -> None:
    group = parser.getgroup("html5lib-tests")
    group.addoption(
        "--dump", type=Path, default=DEFAULT_DUMP, help="path to the html5lib_tokenizer_dump binary"
    )
    group.addoption(
        "--tests-dir", type=Path, default=DEFAULT_TESTS_DIR, help="path to html5lib-tests/tokenizer"
    )
    group.addoption(
        "--excludes", type=Path, default=DEFAULT_EXCLUDES, help="case ids to leave out"
    )
    group.addoption(
        "--include-broken", action="store_true", help="also run the broken-HTML cases"
    )


def pytest_generate_tests(metafunc: pytest.Metafunc) -> None:
    if "case" not in metafunc.fixturenames:
        return
    config = metafunc.config
    tests_dir = config.getoption("--tests-dir")
    if not tests_dir.is_dir():
        raise pytest.UsageError(
            f"html5lib-tests not found at {tests_dir}; configure CMake with "
            "-DARBORIS_HTML5LIB_TESTS=ON, set ARBORIS_BUILD_DIR, or pass --tests-dir"
        )

    cases = load_cases(tests_dir, include_broken=config.getoption("--include-broken"))
    excludes = load_excludes(config.getoption("--excludes"))
    cases = [c for c in cases if c.case_id not in excludes]

    metafunc.parametrize("case", cases, ids=[c.case_id for c in cases])


@pytest.fixture(scope="session")
def dumper(request: pytest.FixtureRequest) -> Iterator[Dumper]:
    binary = request.config.getoption("--dump")
    if not binary.is_file():
        raise pytest.UsageError(
            f"{binary} not found; build the html5lib_tokenizer_dump target or pass --dump"
        )
    instance = Dumper(binary)
    yield instance
    instance.close()
