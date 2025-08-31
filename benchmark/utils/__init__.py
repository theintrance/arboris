"""
Utility functions for benchmarks.

This module contains helper functions and utilities for benchmark operations.
"""

from .utils_compare import (
    load_bytes,
    lxml_parse_and_query,
    selectolax_parse_and_query
)

__all__ = [
    'load_bytes',
    'lxml_parse_and_query', 
    'selectolax_parse_and_query'
]
