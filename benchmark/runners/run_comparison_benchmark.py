import argparse
import json
import sys
import traceback
from pathlib import Path

from benchmark.core import BenchmarkRunner, LxmlParser, SelectolaxParser


def main():
    parser = argparse.ArgumentParser(description="여러 HTML 파서 비교 벤치마크 실행")
    _ = parser.add_argument(
        "--document-type",
        choices=["html", "xml"],
        default="html",
        help="파싱할 문서 타입 (기본값: html)",
    )
    _ = parser.add_argument("--output", type=str, help="결과를 저장할 JSON 파일 경로")
    _ = parser.add_argument(
        "--parsers",
        nargs="+",
        choices=["lxml", "selectolax"],
        default=["lxml", "selectolax"],
        help="비교할 파서들 (기본값: lxml selectolax)",
    )
    _ = parser.add_argument("--verbose", action="store_true", help="상세한 출력 활성화")

    args = parser.parse_args()

    print("HTML 파서 비교 벤치마크 시작")
    print(f"문서 타입: {args.document_type}")
    print(f"비교할 파서들: {', '.join(args.parsers)}")
    print("-" * 50)

    # 파서 생성
    parsers = []
    parser_map = {"lxml": LxmlParser, "selectolax": SelectolaxParser}

    for parser_name in args.parsers:
        if parser_name in parser_map:
            parsers.append(parser_map[parser_name]())
        else:
            print(f"경고: 알 수 없는 파서 '{parser_name}'를 건너뜁니다.")

    if not parsers:
        print("오류: 실행할 파서가 없습니다.")
        sys.exit(1)

    # 벤치마크 실행기 생성
    runner = BenchmarkRunner()

    try:
        # 모든 파서에 대한 벤치마크 실행
        results = runner.run_all_benchmarks(parsers, [args.document_type])

        # 결과 출력
        runner.print_results(results)

        # 결과 저장
        if args.output:
            output_path = Path(args.output)
            output_path.parent.mkdir(parents=True, exist_ok=True)

            all_results = [runner._result_to_dict(result) for result in results]
            with open(output_path, "w", encoding="utf-8") as f:
                json.dump(all_results, f, ensure_ascii=False, indent=2)

            print(f"\n결과가 {output_path}에 저장되었습니다.")

        # 성능 비교 요약
        print("\n" + "=" * 80)
        print("성능 비교 요약")
        print("=" * 80)

        # 파싱 시간 비교
        print("\n평균 파싱 시간 비교:")
        for result in results:
            print(f"  {result.parser_name}: {result.avg_time_ms:.3f} ms")

        # 처리량 비교
        print("\n처리량 비교 (docs/sec):")
        for result in results:
            print(f"  {result.parser_name}: {result.throughput_docs_per_sec:.1f}")

        # 메모리 사용량 비교
        print("\n평균 메모리 사용량 비교:")
        for result in results:
            print(f"  {result.parser_name}: {result.avg_memory_mb:.3f} MB")

        # 성공률 비교
        print("\n성공률 비교:")
        for result in results:
            success_rate = (result.successful_parses / result.total_files) * 100
            print(
                f"  {result.parser_name}: {success_rate:.1f}% ({result.successful_parses}/{result.total_files})"
            )

        print("\n벤치마크 완료!")

    except Exception as e:
        print(f"벤치마크 실행 중 오류 발생: {str(e)}")
        if args.verbose:
            traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
