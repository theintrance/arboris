"""
Core benchmark functionality module.

This module contains the main benchmark runner and parser implementations.
"""

from .benchmark_runner import BenchmarkRunner
from .parsers import BaseParser, LxmlParser, SelectolaxParser

__all__ = [
    'BenchmarkRunner',
    'BaseParser', 
    'LxmlParser',
    'SelectolaxParser'
]
