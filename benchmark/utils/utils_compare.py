from __future__ import annotations

from pathlib import Path

from lxml import html as lxml_html
from selectolax.parser import HTMLParser as SelHTML

UTF8 = "utf-8"
DECODE_ERRORS = "replace"


def load_bytes(path: str | Path) -> bytes:
    return Path(path).read_bytes()


# lxml: XPath 기반 질의
def lxml_parse_and_query(html_bytes: bytes) -> dict[str, object]:
    s = html_bytes.decode(UTF8, errors=DECODE_ERRORS)
    doc = lxml_html.fromstring(s)

    title = doc.xpath("string(//title)")
    meta_desc = doc.xpath(
        "string(//meta[translate(@name,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')='description']/@content)"
    )
    ogs = doc.xpath(
        "//meta[starts-with(translate(@property,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz'),'og:')]/@content"
    )
    a_cnt = len(doc.xpath("//a[@href]"))  # type: ignore
    img_cnt = len(doc.xpath("//img[@src]"))  # type: ignore

    # 스크립트/스타일 제외 + 공백 정규화
    texts = doc.xpath(
        "//text()[normalize-space() and not(ancestor::script) and not(ancestor::style)]"
    )
    text_len = len(" ".join(texts))  # type: ignore

    return {
        "title_len": len(title),  # type: ignore
        "meta_desc_len": len(meta_desc),  # type: ignore
        "og_count": len(ogs),  # type: ignore
        "a_img_count": a_cnt + img_cnt,
        "text_len": text_len,
    }


# selectolax: CSS 기반 질의
def selectolax_parse_and_query(html_bytes: bytes) -> dict[str, object]:
    s = html_bytes.decode(UTF8, errors=DECODE_ERRORS)
    tree = SelHTML(s)

    title = tree.css_first("title")
    meta_desc = tree.css_first("meta[name=description i]")
    ogs = tree.css("meta[property^='og:']")
    a_cnt = len(tree.css("a[href]"))
    img_cnt = len(tree.css("img[src]"))

    body = tree.css_first("body")
    body_text = body.text() if (body and hasattr(body, "text") and body.text is not None) else ""
    body_text = " ".join(body_text.split())

    content = meta_desc.attributes.get("content", "") if meta_desc else ""
    return {
        "title_len": len(title.text()) if title else 0,
        "meta_desc_len": len(content) if content else 0,
        "og_count": len(ogs),
        "a_img_count": a_cnt + img_cnt,
        "text_len": len(body_text),
    }
