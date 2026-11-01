"""arboris: an HTML parser with a queryable DOM.

>>> import arboris
>>> doc = arboris.parse("<ul><li class=a>x<li>y &amp; z</ul>")
>>> [li.text for li in doc.select("ul > li")]
['x', 'y & z']
"""

from ._arboris import Document, Node, __version__, parse

__all__ = ["Document", "Node", "__version__", "parse"]
