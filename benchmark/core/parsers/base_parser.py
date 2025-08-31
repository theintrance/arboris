from abc import ABC, abstractmethod
from typing import Dict, Any, Optional
from pathlib import Path
import time
import psutil
import os
from dataclasses import dataclass

@dataclass
class ParseResult:
  title_length: int = 0
  meta_description_length: int = 0
  og_count: int = 0
  link_count: int = 0
  image_count: int = 0
  text_length: int = 0
  parse_time_ms: float = 0.0
  memory_usage_mb: float = 0.0
  success: bool = True
  error_message: Optional[str] = None

class BaseParser(ABC):    
  def __init__(self, name: str):
    self.name = name
    self.process = psutil.Process(os.getpid())
  
  @abstractmethod
  def parse(self, content: bytes) -> ParseResult:
    pass
  
  def measure_parse(self, content: bytes) -> ParseResult:
    start_memory = self.process.memory_info().rss
    start_time = time.perf_counter()
    
    try:
      result = self.parse(content)
      end_time = time.perf_counter()
      end_memory = self.process.memory_info().rss
      
      result.parse_time_ms = (end_time - start_time) * 1000
      result.memory_usage_mb = (end_memory - start_memory) / (1024 * 1024)
      result.success = True
      
    except Exception as e:
      end_time = time.perf_counter()
      end_memory = self.process.memory_info().rss
      
      result = ParseResult(
        parse_time_ms=(end_time - start_time) * 1000,
        memory_usage_mb=(end_memory - start_memory) / (1024 * 1024),
        success=False,
        error_message=str(e)
      )
    
    return result
  
  def parse_file(self, file_path: Path) -> ParseResult:
    content = file_path.read_bytes()
    return self.measure_parse(content)
