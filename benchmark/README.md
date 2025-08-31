# HTML/XML 파싱 벤치마크

이 디렉토리는 lxml, selectolax의 HTML/XML 파싱 성능을 벤치마크하는 코드를 포함합니다.

## 구조

```
benchmark/
├── core/                     # 핵심 벤치마크 기능
│   ├── __init__.py
│   ├── benchmark_runner.py   # 벤치마크 실행기
│   └── parsers/              # 파서 구현체들
│       ├── __init__.py
│       ├── base_parser.py        # 기본 파서 인터페이스
│       ├── lxml_parser.py        # lxml 파서 구현
│       └── selectolax_parser.py  # selectolax 파서 구현
├── config/                   # 설정 및 의존성
│   ├── __init__.py
│   ├── benchmark_config.py         # 벤치마크 설정 관리
│   └── requirements-benchmark.txt  # 벤치마크 의존성
├── runners/                  # 실행 스크립트들
│   ├── __init__.py
│   ├── run_lxml_benchmark.py       # lxml 벤치마크 실행 스크립트
│   ├── run_comparison_benchmark.py # 여러 파서 비교 벤치마크
│   └── example_usage.py            # 사용 예시 스크립트
├── utils/                    # 유틸리티 함수들
│   ├── __init__.py
│   └── utils_compare.py      # 결과 비교 유틸리티
├── tests/                   # 테스트 및 fixture 파일들
├── results/                 # 벤치마크 결과 저장 디렉토리
├── __init__.py              # 패키지 초기화
└── README.md
```

## 설치

```bash
# 벤치마크 의존성 설치
pip install -r config/requirements-benchmark.txt
```

## 사용법

### 패키지로 import하여 사용

```python
# 전체 패키지에서 필요한 기능만 import
from benchmark import BenchmarkRunner, LxmlParser, SelectolaxParser

# 또는 특정 모듈에서 import
from benchmark.core import BenchmarkRunner
from benchmark.core.parsers import LxmlParser
from benchmark.config import BenchmarkConfig
from benchmark.utils import load_bytes, lxml_parse_and_query, selectolax_parse_and_query
```

### lxml 벤치마크 실행

```bash
# 기본 HTML 벤치마크 실행
python runners/run_lxml_benchmark.py

# XML 벤치마크 실행
python runners/run_lxml_benchmark.py --document-type xml

# 결과를 특정 파일에 저장
python runners/run_lxml_benchmark.py --output results/lxml_benchmark.json

# 상세한 출력 활성화
python runners/run_lxml_benchmark.py --verbose
```

### 여러 파서 비교 벤치마크 실행

```bash
# lxml과 selectolax 비교
python runners/run_comparison_benchmark.py

# 특정 파서들만 비교
python runners/run_comparison_benchmark.py --parsers lxml selectolax

# 결과를 특정 파일에 저장
python runners/run_comparison_benchmark.py --output results/comparison.json
```

### 사용 예시 실행

```bash
# 다양한 사용 예시 확인
python runners/example_usage.py
```

### 프로그래밍 방식으로 벤치마크 실행

```python
from benchmark.core import BenchmarkRunner, LxmlParser

# 파서 생성
lxml_parser = LxmlParser()

# 벤치마크 실행기 생성
runner = BenchmarkRunner()

# 벤치마크 실행
result = runner.run_benchmark(lxml_parser, "html")

# 결과 출력
runner.print_results([result])
```

## 측정 지표

각 벤치마크는 다음 지표들을 측정합니다:

- **파싱 시간**: 평균, 중간값, 95% 백분위수, 99% 백분위수
- **메모리 사용량**: 평균, 최대 메모리 사용량
- **처리량**: 초당 처리 문서 수, 초당 처리 MB 수
- **성공률**: 성공한 파싱 수, 실패한 파싱 수
- **오류 정보**: 발생한 오류들의 상세 정보

## 확장성

### 새로운 파서 추가

1. `core/parsers/` 디렉토리에 새로운 파서 클래스 생성
2. `BaseParser`를 상속받아 `parse()` 메서드 구현
3. `core/parsers/__init__.py`에 새 파서 추가

예시:
```python
from .base_parser import BaseParser, ParseResult

class NewParser(BaseParser):
    def __init__(self):
        super().__init__("new_parser")
    
    def parse(self, content: bytes) -> ParseResult:
        # 파싱 로직 구현
        return ParseResult(...)
```

### 새로운 문서 타입 추가

1. `config/benchmark_config.py`의 `document_types` 리스트에 새 타입 추가
2. fixture 디렉토리에 해당 타입의 테스트 파일 추가

## 결과 분석

벤치마크 결과는 JSON 형식으로 저장되며, 다음과 같은 정보를 포함합니다:

```json
{
  "parser_name": "lxml",
  "document_type": "html",
  "total_files": 10,
  "successful_parses": 9,
  "failed_parses": 1,
  "avg_time_ms": 15.2,
  "throughput_docs_per_sec": 65.8,
  "avg_memory_mb": 2.1,
  "errors": ["file1.html: Invalid HTML structure"]
}
```
