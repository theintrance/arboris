import time
import statistics
from pathlib import Path
from typing import Dict, Any, List, Optional
from dataclasses import dataclass
import json

from config.benchmark_config import config
from .parsers import BaseParser, ParseResult

@dataclass
class BenchmarkResult:
  parser_name: str
  document_type: str
  total_files: int
  successful_parses: int
  failed_parses: int
  total_bytes: int
  total_time_ms: float
  avg_time_ms: float
  median_time_ms: float
  p95_time_ms: float
  p99_time_ms: float
  min_time_ms: float
  max_time_ms: float
  avg_memory_mb: float
  max_memory_mb: float
  throughput_docs_per_sec: float
  throughput_mb_per_sec: float
  errors: List[str]

class BenchmarkRunner:    
  def __init__(self):
    self.results: List[BenchmarkResult] = []
  
  def run_benchmark(self, parser: BaseParser, document_type: str = "html") -> BenchmarkResult:
    print(f"벤치마크 실행 중: {parser.name} ({document_type})")
    
    # fixture 파일들 가져오기
    files = config.get_fixture_files(document_type)
    if not files:
      raise ValueError(f"문서 타입 '{document_type}'에 대한 fixture 파일을 찾을 수 없습니다.")
    
    print(f"  - {len(files)}개의 파일을 찾았습니다.")
    
    # 벤치마크 실행
    parse_times = []
    memory_usage = []
    errors = []
    total_bytes = 0
    successful_parses = 0
    failed_parses = 0
    
    for i, file_path in enumerate(files):
      print(f"  - 진행률: {i+1}/{len(files)} ({file_path.name})")
      
      try:
        result = parser.parse_file(file_path)
        total_bytes += file_path.stat().st_size
        
        if result.success:
          successful_parses += 1
          parse_times.append(result.parse_time_ms)
          memory_usage.append(result.memory_usage_mb)
        else:
          failed_parses += 1
          errors.append(f"{file_path.name}: {result.error_message}")
          
      except Exception as e:
        failed_parses += 1
        errors.append(f"{file_path.name}: {str(e)}")
    
    # 통계 계산
    if parse_times:
      total_time = sum(parse_times)
      avg_time = statistics.mean(parse_times)
      median_time = statistics.median(parse_times)
      min_time = min(parse_times)
      max_time = max(parse_times)
      
      # 백분위수 계산
      sorted_times = sorted(parse_times)
      p95_idx = int(len(sorted_times) * 0.95)
      p99_idx = int(len(sorted_times) * 0.99)
      p95_time = sorted_times[p95_idx] if p95_idx < len(sorted_times) else max_time
      p99_time = sorted_times[p99_idx] if p99_idx < len(sorted_times) else max_time
      
      # 처리량 계산
      throughput_docs_per_sec = successful_parses / (total_time / 1000) if total_time > 0 else 0
      throughput_mb_per_sec = (total_bytes / (1024 * 1024)) / (total_time / 1000) if total_time > 0 else 0
      
      # 메모리 통계
      avg_memory = statistics.mean(memory_usage) if memory_usage else 0
      max_memory = max(memory_usage) if memory_usage else 0
    else:
      total_time = avg_time = median_time = p95_time = p99_time = min_time = max_time = 0
      throughput_docs_per_sec = throughput_mb_per_sec = 0
      avg_memory = max_memory = 0
    
    # 결과 생성
    benchmark_result = BenchmarkResult(
      parser_name=parser.name,
      document_type=document_type,
      total_files=len(files),
      successful_parses=successful_parses,
      failed_parses=failed_parses,
      total_bytes=total_bytes,
      total_time_ms=total_time,
      avg_time_ms=avg_time,
      median_time_ms=median_time,
      p95_time_ms=p95_time,
      p99_time_ms=p99_time,
      min_time_ms=min_time,
      max_time_ms=max_time,
      avg_memory_mb=avg_memory,
      max_memory_mb=max_memory,
      throughput_docs_per_sec=throughput_docs_per_sec,
      throughput_mb_per_sec=throughput_mb_per_sec,
      errors=errors
    )
    
    self.results.append(benchmark_result)
    return benchmark_result
    
  def run_all_benchmarks(self, parsers: List[BaseParser], document_types: Optional[List[str]] = None) -> List[BenchmarkResult]:
    if document_types is None:
      document_types = config.document_types
    
    all_results = []
    
    for parser in parsers:
      for doc_type in document_types:
        try:
          result = self.run_benchmark(parser, doc_type)
          all_results.append(result)
          
          # 결과 저장
          config.save_results(self._result_to_dict(result), parser.name, doc_type)
          
        except Exception as e:
          print(f"벤치마크 실패: {parser.name} ({doc_type}) - {str(e)}")
    
    return all_results
  
  def _result_to_dict(self, result: BenchmarkResult) -> Dict[str, Any]:
    return {
      "parser_name": result.parser_name,
      "document_type": result.document_type,
      "total_files": result.total_files,
      "successful_parses": result.successful_parses,
      "failed_parses": result.failed_parses,
      "total_bytes": result.total_bytes,
      "total_time_ms": result.total_time_ms,
      "avg_time_ms": result.avg_time_ms,
      "median_time_ms": result.median_time_ms,
      "p95_time_ms": result.p95_time_ms,
      "p99_time_ms": result.p99_time_ms,
      "min_time_ms": result.min_time_ms,
      "max_time_ms": result.max_time_ms,
      "avg_memory_mb": result.avg_memory_mb,
      "max_memory_mb": result.max_memory_mb,
      "throughput_docs_per_sec": result.throughput_docs_per_sec,
      "throughput_mb_per_sec": result.throughput_mb_per_sec,
      "errors": result.errors
    }
  
  def print_results(self, results: Optional[List[BenchmarkResult]] = None):
    if results is None:
      results = self.results
    
    print("\n" + "="*80)
    print("벤치마크 결과 요약")
    print("="*80)
    
    for result in results:
      print(f"\n{result.parser_name} ({result.document_type})")
      print(f"  파일 수: {result.total_files} (성공: {result.successful_parses}, 실패: {result.failed_parses})")
      print(f"  총 크기: {result.total_bytes / (1024*1024):.2f} MB")
      print(f"  평균 파싱 시간: {result.avg_time_ms:.2f} ms")
      print(f"  중간값 파싱 시간: {result.median_time_ms:.2f} ms")
      print(f"  95% 파싱 시간: {result.p95_time_ms:.2f} ms")
      print(f"  처리량: {result.throughput_docs_per_sec:.2f} docs/sec, {result.throughput_mb_per_sec:.2f} MB/sec")
      print(f"  평균 메모리 사용량: {result.avg_memory_mb:.2f} MB")
      print(f"  최대 메모리 사용량: {result.max_memory_mb:.2f} MB")
      
      if result.errors:
        print(f"  오류 수: {len(result.errors)}")
        for error in result.errors[:3]:  # 처음 3개만 표시
          print(f"    - {error}")
        if len(result.errors) > 3:
          print(f"    ... 및 {len(result.errors) - 3}개 더")
