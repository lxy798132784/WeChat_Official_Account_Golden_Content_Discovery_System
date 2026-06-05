#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" -j"${JOBS:-2}"
ctest --test-dir "$ROOT/build" --output-on-failure
QT_QPA_PLATFORM=offscreen "$ROOT/build/premium-content-radar" --self-test
QT_QPA_PLATFORM=offscreen "$ROOT/build/premium-content-radar" --screenshot "$ROOT/docs/assets/preview-runtime.png"
test -s "$ROOT/docs/assets/preview-runtime.png"
python3 "$ROOT/scripts/audit_no_forbidden_tokens.py"
python3 "$ROOT/scripts/audit_requirements.py"
printf 'OK: build, tests, self-test, screenshot, requirements audit, and secret audit passed\n'
