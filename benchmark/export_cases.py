"""Write the html5lib-tests tokenizer cases as arboris_bench input.

Each case becomes a header line and its input bytes:

  <case id>\\t<area>\\t<feature>\\t<1 if arboris passes it, else 0>\\t<byte length>\\n<input>\\n

Cases, exclusions and areas come from tests/html5lib, so the benchmark runs the same cases
the conformance test does.

Usage:
  uv run --group test python benchmark/export_cases.py --dump <html5lib_tokenizer_dump> \\
      --tests-dir <html5lib-tests/tokenizer> --output <file>
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tests" / "html5lib"))

from categories import classify  # noqa: E402
from conftest import (  # noqa: E402
    DEFAULT_EXCLUDES,
    DEFAULT_TESTS_DIR,
    Case,
    Dumper,
    load_cases,
    load_excludes,
    normalize,
)


def slug(name: str) -> str:
    return re.sub(r"[^0-9a-z]+", "-", name.lower()).strip("-")


def passes(dumper: Dumper, case: Case) -> bool:
    result = dumper.tokenize(case.input)
    return bool(result["ok"]) and normalize(result["tokens"]) == case.expected


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("--dump", type=Path, required=True)
    parser.add_argument("--tests-dir", type=Path, default=DEFAULT_TESTS_DIR)
    parser.add_argument("--excludes", type=Path, default=DEFAULT_EXCLUDES)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    excludes = load_excludes(args.excludes)
    cases = [c for c in load_cases(args.tests_dir) if c.case_id not in excludes]

    dumper = Dumper(args.dump)
    out = bytearray()
    try:
        for case in cases:
            area = classify(case)
            data = case.input.encode("utf-8", "surrogatepass")
            header = [case.case_id, slug(area.major), slug(area.feature)]
            header += [str(int(passes(dumper, case))), str(len(data))]
            out += "\t".join(header).encode() + b"\n" + data + b"\n"
    finally:
        dumper.close()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(out)


if __name__ == "__main__":
    main()
