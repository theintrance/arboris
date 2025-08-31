#!/bin/bash

set -e

# cpplint
uv run cpplint --quiet --recursive --config=CPPLINT.cfg .

# clang-tidy
# TODO(jayden): add clang-tidy
