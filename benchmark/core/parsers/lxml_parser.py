from typing import List
from lxml import html as lxml_html
from .base_parser import BaseParser, ParseResult


class LxmlParser(BaseParser):
    def __init__(self):
        super().__init__("lxml")

    def parse(self, content: bytes) -> ParseResult:
        # UTF-8로 디코딩 (에러 처리 포함)
        try:
            html_str = content.decode("utf-8", errors="replace")
        except UnicodeDecodeError:
            html_str = content.decode("latin-1", errors="replace")

        # lxml로 파싱
        doc = lxml_html.fromstring(html_str)

        # 제목 추출
        title_elements = doc.xpath("//title")
        title_text = title_elements[0].text if title_elements else ""

        # 메타 설명 추출 (대소문자 구분 없이)
        meta_desc_elements = doc.xpath(
            "//meta[translate(@name,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')='description']/@content"
        )
        meta_desc = meta_desc_elements[0] if meta_desc_elements else ""

        # Open Graph 태그 개수
        og_elements = doc.xpath(
            "//meta[starts-with(translate(@property,'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz'),'og:')]"
        )

        # 링크 개수
        link_elements = doc.xpath("//a[@href]")

        # 이미지 개수
        img_elements = doc.xpath("//img[@src]")

        # 텍스트 추출 (스크립트/스타일 제외)
        text_elements = doc.xpath(
            "//text()[normalize-space() and not(ancestor::script) and not(ancestor::style)]"
        )
        text_content = " ".join(text_elements)

        return ParseResult(
            title_length=len(title_text),
            meta_description_length=len(meta_desc),
            og_count=len(og_elements),
            link_count=len(link_elements),
            image_count=len(img_elements),
            text_length=len(text_content),
        )

    def parse_xml(self, content: bytes) -> ParseResult:
        # XML 파싱은 HTML 파싱과 유사하지만 더 엄격한 규칙 적용
        return self.parse(content)
