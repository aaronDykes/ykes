#!/usr/bin/env bash
set -euo pipefail

# Local CI helper: build with ASan and run tests (no Docker/Valgrind).
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

echo "Building with AddressSanitizer (Linux-style flags)."
make clean || true
make CC=clang CFLAGS='-g -O1 -fsanitize=address -fno-omit-frame-pointer -Wall -Wextra -pedantic'

echo "Running tests via tests/run_tests.sh"
chmod +x tests/run_tests.sh
./tests/run_tests.sh

echo "Local CI complete. To run valgrind use the Docker image: Dockerfile.valgrind."
