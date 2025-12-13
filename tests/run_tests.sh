#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

mkdir -p tests/actual tests/expected

FAILED=0

echo "Running Ykes tests..."
for f in test/*.yk; do
  [ -f "$f" ] || continue
  name=$(basename "$f" .yk)
  out="tests/actual/${name}.out"
  echo "- Running $f -> $out"
  ./ykes "$f" > "$out" 2>&1 || true

  expected="tests/expected/${name}.out"
  if [ -f "$expected" ]; then
    if ! diff -u "$expected" "$out" >/dev/null; then
      echo "  -> FAIL: output differs (see diff)"
      diff -u "$expected" "$out" || true
      FAILED=1
    else
      echo "  -> OK"
    fi
  else
    echo "  -> No expected output found. Creating $expected (baseline)."
    cp "$out" "$expected"
  fi
done

if [ "$FAILED" -ne 0 ]; then
  echo "Some tests failed. See tests/actual for outputs."
  exit 1
fi

echo "All tests passed or baselines created."
