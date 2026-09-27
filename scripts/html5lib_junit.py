"""Read case results out of the JUnit XML pytest writes for the tokenizer suite.

Shared by compare-html5lib-results.py and html5lib_status.py so the CI report and the
README table agree on what counts as a passing case.
"""

from __future__ import annotations

import re
import xml.etree.ElementTree as ET
from pathlib import Path

CASE_ID_RE = re.compile(r"\[(.+)\]$")
NOT_RUN = {"failure", "error", "skipped"}


def load_results(path: Path) -> dict[str, bool]:
    """Map every case id in a JUnit XML file to whether it passed."""
    results: dict[str, bool] = {}
    for testcase in ET.parse(path).getroot().iter("testcase"):
        match = CASE_ID_RE.search(testcase.get("name", ""))
        if match is None:
            continue
        results[match.group(1)] = all(child.tag not in NOT_RUN for child in testcase)
    return results
