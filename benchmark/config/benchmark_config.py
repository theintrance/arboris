import json
from pathlib import Path
from typing import Any, final


@final
class BenchmarkConfig:
    def __init__(self):
        self.fixtures_dir = Path(__file__).parent.parent / "tests" / "conformance" / "fixtures"
        self.results_dir = Path(__file__).parent.parent / "results"
        self.results_dir.mkdir(exist_ok=True)

        # 지원하는 파서 타입
        self.parser_types = ["lxml", "selectolax", "beautifulsoup", "html5lib"]

        # 지원하는 문서 타입
        self.document_types = ["html", "xml"]

        # 벤치마크 설정
        self.benchmark_settings = {
            "iterations": 100,
            "warmup_iterations": 10,
            "timeout_seconds": 30,
            "memory_tracking": True,
            "cpu_tracking": True,
        }

    def get_fixture_files(self, doc_type: str = "html") -> list[Path]:
        pattern = f"*.{doc_type}"
        files: list[Path] = []

        for fixture_dir in self.fixtures_dir.iterdir():
            if fixture_dir.is_dir():
                files.extend(fixture_dir.glob(pattern))

        return files

    def get_result_path(self, parser: str, doc_type: str) -> Path:
        return self.results_dir / f"benchmark_{parser}_{doc_type}.json"

    def save_results(self, results: dict[str, Any], parser: str, doc_type: str):
        result_path = self.get_result_path(parser, doc_type)
        with open(result_path, "w", encoding="utf-8") as f:
            json.dump(results, f, ensure_ascii=False, indent=2)

    def load_results(self, parser: str, doc_type: str) -> dict[str, object]:
        result_path = self.get_result_path(parser, doc_type)
        if result_path.exists():
            with open(result_path, "r", encoding="utf-8") as f:
                return json.load(f)
        return {}


# 전역 설정 인스턴스
config = BenchmarkConfig()
