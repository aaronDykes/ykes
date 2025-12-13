#!/usr/bin/env bash
set -euo pipefail

# Run valgrind over the repo's example/test files and write logs to ./valgrind-logs
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

mkdir -p valgrind-logs

echo "Building project (non-sanitized)"
make clean || true
make CC=clang CFLAGS="-g -O0 -Wall -Wextra -pedantic"

echo "Searching for .yk test files"
find test control_flow data_structures file leet -type f -name '*.yk' -print0 2>/dev/null | \
  while IFS= read -r -d '' f; do
    safe_name=$(echo "$f" | sed 's#[/ ]#_#g')
    log="valgrind-logs/valgrind_${safe_name}.log"
    echo "== Running valgrind on: $f"
    # Run valgrind, allow program to exit non-zero but capture log
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file="$log" ./ykes "$f" || true
    echo "  -> log: $log"
  done

echo "Valgrind run complete. Logs in: $ROOT_DIR/valgrind-logs"
