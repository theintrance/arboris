#!/bin/bash

set -e

function set_git_hooks_path() {
    git config core.hooksPath hooks
}

function install_python_dependencies() {
    uv sync
}

set_git_hooks_path
install_python_dependencies
