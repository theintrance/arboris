#!/usr/bin/env python3
"""
lxml HTML 파싱 벤치마크 실행 스크립트
"""
import sys
import argparse
from pathlib import Path

# 프로젝트 루트를 Python 경로에 추가
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

from core import BenchmarkRunner, LxmlParser

def main():
  parser = argparse.ArgumentParser(description="lxml HTML 파싱 벤치마크 실행")
  parser.add_argument(
    "--document-type", 
    choices=["html", "xml"], 
    default="html",
    help="파싱할 문서 타입 (기본값: html)"
  )
  parser.add_argument(
    "--output", 
    type=str,
    help="결과를 저장할 JSON 파일 경로"
  )
  parser.add_argument(
    "--verbose", 
    action="store_true",
    help="상세한 출력 활성화"
  )
  
  args = parser.parse_args()
  
  print("lxml HTML 파싱 벤치마크 시작")
  print(f"문서 타입: {args.document_type}")
  print("-" * 50)
  
  # lxml 파서 생성
  lxml_parser = LxmlParser()
  
  # 벤치마크 실행기 생성
  runner = BenchmarkRunner()
  
  try:
    # 벤치마크 실행
    result = runner.run_benchmark(lxml_parser, args.document_type)
    
    # 결과 출력
    runner.print_results([result])
    
    # 결과 저장
    if args.output:
      output_path = Path(args.output)
      output_path.parent.mkdir(parents=True, exist_ok=True)
      
      import json
      with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(runner._result_to_dict(result), f, ensure_ascii=False, indent=2)
      
      print(f"\n결과가 {output_path}에 저장되었습니다.")
    
    print("\n벤치마크 완료!")
    
  except Exception as e:
    print(f"벤치마크 실행 중 오류 발생: {str(e)}")
    if args.verbose:
      import traceback
      traceback.print_exc()
    sys.exit(1)

if __name__ == "__main__":
    main()
