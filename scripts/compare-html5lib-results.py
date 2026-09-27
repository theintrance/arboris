"""Compare two html5lib-tests tokenizer runs and report what changed.

Reads the JUnit XML that pytest writes for the base branch and for the pull request,
then lists the cases that started passing and the ones that stopped passing. Exits 1
when a case regressed, so CI fails on a regression but not on remaining work.

Usage:
  compare-html5lib-results.py --base base.xml --head head.xml [--limit 30]
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from html5lib_junit import load_results


def format_list(title: str, case_ids: list[str], limit: int) -> list[str]:
    lines = [f"### {title} ({len(case_ids)})", ""]
    if not case_ids:
        lines += ["_none_", ""]
        return lines
    lines += ["```"]
    lines += case_ids[:limit]
    if len(case_ids) > limit:
        lines += [f"... and {len(case_ids) - limit} more"]
    lines += ["```", ""]
    return lines


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("--base", type=Path, required=True, help="JUnit XML of the base branch")
    parser.add_argument("--head", type=Path, required=True, help="JUnit XML of the pull request")
    parser.add_argument("--limit", type=int, default=30, help="case ids to list per section")
    args = parser.parse_args()

    base = load_results(args.base)
    head = load_results(args.head)

    fixed = sorted(c for c, ok in head.items() if ok and not base.get(c, False))
    broken = sorted(c for c, ok in head.items() if not ok and base.get(c, False))
    added = sorted(set(head) - set(base))
    removed = sorted(set(base) - set(head))

    base_pass = sum(base.values())
    head_pass = sum(head.values())
    delta = head_pass - base_pass

    lines = [
        "## html5lib-tests tokenizer",
        "",
        f"Passing: **{base_pass} / {len(base)}** (base) → **{head_pass} / {len(head)}** "
        f"({delta:+d})",
        "",
    ]
    lines += format_list("✅ Newly passing", fixed, args.limit)
    lines += format_list("❌ Newly failing", broken, args.limit)
    if added:
        lines += format_list("➕ New cases", added, args.limit)
    if removed:
        lines += format_list("➖ Dropped cases", removed, args.limit)
    if broken:
        lines += ["**These cases passed on the base branch. This pull request broke them.**", ""]

    print("\n".join(lines))
    return 1 if broken else 0


if __name__ == "__main__":
    sys.exit(main())
