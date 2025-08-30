from __future__ import annotations
import pytest
from pathlib import Path

from pathlib import Path
from utils.utils_compare import (
    load_bytes, lxml_parse_and_query, selectolax_parse_and_query
)

# fixture 디렉토리 경로
FIXTURES_DIR = Path(__file__).parent / "fixtures"

EPS = {
    "title_len": 2,
    "meta_desc_len": 16,
    "og_count": 1,
    "a_img_count": 2,
    "text_len": 2048,
}

def _file(rel: str) -> Path:
    return (FIXTURES_DIR / rel).resolve()

@pytest.mark.parametrize("rel", [
  "static/tiny_static.html",
  "dynamic/tiny_dynamic.html",
])
def test_equivalence_small(rel: str):
  b = load_bytes(_file(rel))
  a = lxml_parse_and_query(b)
  c = selectolax_parse_and_query(b)

  assert abs(a["title_len"] - c["title_len"]) <= EPS["title_len"]
  assert abs(a["meta_desc_len"] - c["meta_desc_len"]) <= EPS["meta_desc_len"]
  assert abs(a["og_count"] - c["og_count"]) <= EPS["og_count"]
  assert abs(a["a_img_count"] - c["a_img_count"]) <= EPS["a_img_count"]
  assert abs(a["text_len"] - c["text_len"]) <= EPS["text_len"]

def test_broken_html_no_crash():
  b = load_bytes(_file("broken/tiny_broken.html"))
  # 예외 없이 처리되는지만 확인
  lxml_parse_and_query(b)
  selectolax_parse_and_query(b)