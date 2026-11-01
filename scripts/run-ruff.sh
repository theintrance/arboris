#!/bin/bash

set -e

if ! which uv > /dev/null; then
    echo "uv is not installed"
    exit 1
fi

ROOT=$(git rev-parse --show-toplevel)

cd "$ROOT"

uv run --no-project --with ruff==0.12.11 ruff check "$ROOT"
