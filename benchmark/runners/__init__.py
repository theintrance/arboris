"""
Benchmark runner scripts module.

This module contains scripts for running different types of benchmarks.
"""

from .example_usage import main as example_main
from .run_comparison_benchmark import main as comparison_main
from .run_lxml_benchmark import main as lxml_main

__all__ = [
    'example_main',
    'comparison_main', 
    'lxml_main'
]
