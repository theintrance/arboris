#!/bin/bash

# Runs clang-tidy over every source in src/ against the compile database in $BUILD_DIR
# (default: build); headers are checked through the sources that include them. Configure
# with cmake first; CMAKE_EXPORT_COMPILE_COMMANDS is on.

set -o pipefail

ROOT=$(git rev-parse --show-toplevel)
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"

if ! which clang-tidy > /dev/null; then
  echo "clang-tidy is not installed"
  exit 1
fi

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
  echo "No compile database in $BUILD_DIR, please run cmake first"
  exit 1
fi

EXTRA_ARGS=(--extra-arg=-std=c++20)
# A clang-tidy that is not Apple's (Homebrew LLVM) does not know where the macOS SDK is.
if [ "$(uname)" = "Darwin" ] && which xcrun > /dev/null; then
  EXTRA_ARGS+=(--extra-arg=-isysroot --extra-arg="$(xcrun --show-sdk-path)")
fi

cd "$ROOT" || exit 1

JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
find src -name "*.cc" -print0 |
  xargs -0 -P "$JOBS" -n 4 clang-tidy -p "$BUILD_DIR" --quiet --warnings-as-errors='*' "${EXTRA_ARGS[@]}"
STATUS=$?

if [ "$STATUS" -ne 0 ]; then
  echo "clang-tidy failed"
  exit 1
fi

echo "All checks passed"
exit 0
