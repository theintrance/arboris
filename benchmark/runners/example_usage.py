#!/usr/bin/env python3
import sys
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

from core import BenchmarkRunner, LxmlParser, SelectolaxParser
from config import config


def example_single_parser():
    print("=== 단일 파서 벤치마크 예시 ===")

    # lxml 파서 생성
    lxml_parser = LxmlParser()

    # 벤치마크 실행기 생성
    runner = BenchmarkRunner()

    # HTML 파일에 대한 벤치마크 실행
    result = runner.run_benchmark(lxml_parser, "html")

    # 결과 출력
    runner.print_results([result])

    return result


def example_multiple_parsers():
    print("\n=== 여러 파서 비교 벤치마크 예시 ===")

    # 여러 파서 생성
    parsers = [LxmlParser(), SelectolaxParser()]

    # 벤치마크 실행기 생성
    runner = BenchmarkRunner()

    results = runner.run_all_benchmarks(parsers, ["html"])

    # 결과 출력
    runner.print_results(results)

    return results


def example_custom_analysis():
    print("\n=== 커스텀 분석 예시 ===")

    # 벤치마크 실행
    results = example_multiple_parsers()

    # 성능 순위 계산
    sorted_by_speed = sorted(results, key=lambda r: r.avg_time_ms)
    sorted_by_throughput = sorted(results, key=lambda r: r.throughput_docs_per_sec, reverse=True)
    sorted_by_memory = sorted(results, key=lambda r: r.avg_memory_mb)

    print("\n성능 순위 (파싱 시간 기준 - 빠른 순):")
    for i, result in enumerate(sorted_by_speed, 1):
        print(f"  {i}. {result.parser_name}: {result.avg_time_ms:.3f} ms")

    print("\n처리량 순위 (docs/sec 기준 - 높은 순):")
    for i, result in enumerate(sorted_by_throughput, 1):
        print(f"  {i}. {result.parser_name}: {result.throughput_docs_per_sec:.1f} docs/sec")

    print("\n메모리 효율성 순위 (메모리 사용량 기준 - 낮은 순):")
    for i, result in enumerate(sorted_by_memory, 1):
        print(f"  {i}. {result.parser_name}: {result.avg_memory_mb:.3f} MB")


def example_config_usage():
    print("\n=== 설정 사용 예시 ===")

    # fixture 파일들 가져오기
    html_files = config.get_fixture_files("html")
    print(f"HTML fixture 파일 수: {len(html_files)}")

    for file_path in html_files:
        print(f"  - {file_path.name}")

    # 설정 정보 출력
    print(f"\n지원하는 파서 타입: {config.parser_types}")
    print(f"지원하는 문서 타입: {config.document_types}")
    print(f"벤치마크 설정: {config.benchmark_settings}")


if __name__ == "__main__":
    print("벤치마크 시스템 사용 예시")
    print("=" * 50)

    try:
        # 예시 실행
        example_config_usage()
        example_single_parser()
        example_multiple_parsers()
        example_custom_analysis()

        print("\n모든 예시가 성공적으로 실행되었습니다!")

    except Exception as e:
        print(f"예시 실행 중 오류 발생: {str(e)}")
        import traceback

        traceback.print_exc()
