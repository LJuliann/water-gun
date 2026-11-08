#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$project_dir/tests" -B "$project_dir/build/host-tests" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build "$project_dir/build/host-tests"
ctest --test-dir "$project_dir/build/host-tests" --output-on-failure
