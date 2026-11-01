import pytest

import arboris

PAGE = (
    "<html><body>"
    "<ul id=l><li class='a b'>x<li>y &amp; z</ul>"
    "<p id=p data-k=v hidden>t</p>"
    "</body></html>"
)


@pytest.fixture
def doc():
    return arboris.parse(PAGE)


def test_parse_reads_str_as_utf8(doc):
    assert doc.encoding == "utf-8"
    assert doc.parse_error_offset is None
    assert doc.unsupported_encoding is None
    assert doc.html == PAGE


def test_parse_decodes_bytes_by_meta():
    page = arboris.parse(b"<meta charset=euc-kr><p>\xc7\xd1\xb1\xdb</p>")
    if page.unsupported_encoding == "euc-kr":
        pytest.skip("built without iconv, so only UTF-8 and UTF-16 are decoded")
    assert page.encoding == "euc-kr"
    assert page.select_one("p").text == "한글"


def test_parse_rejects_other_types():
    with pytest.raises(TypeError):
        arboris.parse(3)


def test_select_returns_document_order(doc):
    items = doc.select("ul > li")
    assert [li.text for li in items] == ["x", "y & z"]
    assert items[0].classes == ["a", "b"]
    assert items[1].outer_html == "<li>y &amp; z"
    assert doc.select_one("#p").tag == "p"
    assert doc.select_one("nothing") is None
    assert doc.select("nothing") == []


def test_select_rejects_text_that_is_not_a_selector(doc):
    with pytest.raises(ValueError):
        doc.select("div >")


def test_find_conditions(doc):
    assert doc.find(tag="li", classes="a").text == "x"
    assert [n.text for n in doc.find_all(classes=["a", "b"])] == ["x"]
    assert doc.find(attrs={"data-k": "v"}).id == "p"
    assert doc.find(attrs={"hidden": None}).id == "p"
    assert doc.find(attrs={"data-k": "other"}) is None
    assert doc.find(text="t").id == "p"
    assert [n.tag for n in doc.find_all(text=lambda t: "&" in t)] == ["html", "body", "ul", "li"]
    assert doc.get_by_id("p").attrs == {"id": "p", "data-k": "v", "hidden": ""}
    assert doc.get_by_id("missing") is None


def test_queries_from_a_node_search_its_descendants(doc):
    ul = doc.select_one("ul")
    assert [li.text for li in ul.select("li")] == ["x", "y & z"]
    assert [li.text for li in ul.find_all(tag="li")] == ["x", "y & z"]
    assert ul.select_one("p") is None
    assert ul.select_one("ul") is None


def test_navigation(doc):
    first, second = doc.select("li")
    assert second.parent.id == "l"
    assert second.previous_sibling == first
    assert second.next_sibling is None
    assert [n.tag for n in second.ancestors] == ["ul", "body", "html"]
    assert second.parent.children == [first, second]
    assert doc.root.parent is None
    assert doc.root.ancestors == []
    assert doc.root.children[0].tag == "html"


def test_nodes_compare_by_identity_within_a_document(doc):
    a = doc.select_one("#p")
    b = doc.get_by_id("p")
    assert a == b
    assert hash(a) == hash(b)
    assert a != doc.select_one("ul")
    assert len({a, b}) == 1
    assert a != arboris.parse(PAGE).select_one("#p")


def test_inner_and_outer_html(doc):
    p = doc.select_one("#p")
    assert p.outer_html == "<p id=p data-k=v hidden>t</p>"
    assert p.inner_html == "t"
    assert doc.select_one("ul").inner_html == "<li class='a b'>x<li>y &amp; z"


def test_nodes_keep_their_document_alive():
    node = arboris.parse("<p>keep me</p>").select_one("p")
    assert node.text == "keep me"


def test_repr(doc):
    assert repr(doc.select_one("#p")) == '<arboris.Node p id="p">'
    assert repr(doc.root) == "<arboris.Node #document>"
    assert repr(doc).startswith("<arboris.Document ")


def test_version():
    assert arboris.__version__ == "0.1.0"
