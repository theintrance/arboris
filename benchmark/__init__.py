"""
Arboris Benchmark Suite

A comprehensive benchmarking framework for HTML parsing libraries.
"""

__version__ = "1.0.0"

# Import main components
from .core import BenchmarkRunner, BaseParser, LxmlParser, SelectolaxParser
from .config import BenchmarkConfig
from .utils import load_bytes, lxml_parse_and_query, selectolax_parse_and_query

__all__ = [
    'BenchmarkRunner',
    'BaseParser',
    'LxmlParser', 
    'SelectolaxParser',
    'BenchmarkConfig',
    'load_bytes',
    'lxml_parse_and_query',
    'selectolax_parse_and_query'
]
