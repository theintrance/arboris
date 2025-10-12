#!/bin/bash

ROOT=$(git rev-parse --show-toplevel)
DEBUG_BUILD_DIR="$ROOT/cmake-build-debug"
RELEASE_BUILD_DIR="$ROOT/cmake-build-release"
TIDY_TARGET_DIRS=("$ROOT/include" "$ROOT/src")
PROCESSED=false
TIDY_PASSED=false

function run_clang_tidy() {
  BUILD_DIR="$1"
  for TARGET_DIR in "${TIDY_TARGET_DIRS[@]}"; do
    while read -r FILE; do
      if ! clang-tidy "$FILE" -p "$BUILD_DIR" --warnings-as-errors=* --extra-arg=-std=c++20; then
        TIDY_PASSED=false
      fi
    done < <(find "$TARGET_DIR" -name "*.cc" -o -name "*.hpp")
  done
}

cd "$ROOT" || exit 1

if [ -d "$DEBUG_BUILD_DIR" ]; then
  run_clang_tidy "$DEBUG_BUILD_DIR"
  PROCESSED=true
fi

if [ -d "$RELEASE_BUILD_DIR" ]; then
  run_clang_tidy "$RELEASE_BUILD_DIR"
  PROCESSED=true
fi

if [ "$PROCESSED" = false ]; then
  echo "No build directory found, please run cmake first"
  exit 1
fi

if [ "$TIDY_PASSED" = false ]; then
  echo "Clang-tidy failed"
  exit 1
fi

echo "All checks passed"
exit 0