from selectolax.parser import HTMLParser as SelHTML
from .base_parser import BaseParser, ParseResult


class SelectolaxParser(BaseParser):
    def __init__(self):
        super().__init__("selectolax")

    def parse(self, content: bytes) -> ParseResult:
        # UTF-8로 디코딩 (에러 처리 포함)
        try:
            html_str = content.decode("utf-8", errors="replace")
        except UnicodeDecodeError:
            html_str = content.decode("latin-1", errors="replace")

        tree = SelHTML(html_str)

        # 제목 추출
        title_element = tree.css_first("title")
        title_text = title_element.text() if title_element else ""

        # 메타 설명 추출 (대소문자 구분 없이)
        meta_desc_element = tree.css_first("meta[name='description']")
        meta_desc = meta_desc_element.attributes.get("content", "") if meta_desc_element else ""

        # Open Graph 태그 개수
        og_elements = tree.css("meta[property^='og:']")

        # 링크 개수
        link_elements = tree.css("a[href]")

        # 이미지 개수
        img_elements = tree.css("img[src]")

        # 텍스트 추출 (body 내의 텍스트)
        body_element = tree.css_first("body")
        if body_element and hasattr(body_element, "text") and body_element.text:
            text_content = " ".join(body_element.text().split())
        else:
            text_content = ""

        return ParseResult(
            title_length=len(title_text),
            meta_description_length=len(meta_desc),
            og_count=len(og_elements),
            link_count=len(link_elements),
            image_count=len(img_elements),
            text_length=len(text_content),
        )
