#!/usr/bin/env bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BIN="$ROOT/bin/mini-ccstatus"

PASS=0
FAIL=0

if [[ ! -x "$BIN" ]]; then
  echo "error: missing binary '$BIN'"
  exit 1
fi

# Colors for test output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

test_passed() {
  echo -e "${GREEN}✓${NC} $1"
  PASS=$((PASS + 1))
}

test_failed() {
  echo -e "${RED}✗${NC} $1"
  FAIL=$((FAIL + 1))
}

# Test: Input size limit - 1MB should pass size check
test_max_input_size_at_limit() {
  local exit_code=0
  local output
  # Generate exactly 1MB (1048576 bytes) of input
  output="$(dd if=/dev/zero bs=1M count=1 2>/dev/null | tr '\0' 'x' | head -c 1048576 | NO_COLOR=1 "$BIN" 2>&1)" || exit_code=$?

  # Should NOT trigger size limit error (exit code would be 3 for ERROR_IO)
  # Will fail with ERROR_JSON (4) since it's not valid JSON, but that's expected
  if [[ "$exit_code" -eq 4 ]] && echo "$output" | grep -q "invalid JSON"; then
    test_passed "Input at 1MB limit passes size check"
  else
    test_failed "Input at 1MB limit passes size check"
    echo "  expected exit code 4 (ERROR_JSON), got $exit_code"
    echo "  output: $output"
  fi
}

# Test: Input size limit - over 1MB should fail
test_max_input_size_exceeded() {
  local exit_code=0
  local output
  # Generate 2MB of input to exceed the limit
  output="$(dd if=/dev/zero bs=2M count=1 2>/dev/null | tr '\0' 'x' | head -c 2097152 | NO_COLOR=1 "$BIN" 2>&1)" || exit_code=$?

  # Should trigger size limit error (exit code 3 for ERROR_IO)
  if [[ "$exit_code" -eq 3 ]] && echo "$output" | grep -q "exceeds maximum size limit"; then
    test_passed "Input over 1MB rejected with size limit error"
  else
    test_failed "Input over 1MB rejected with size limit error"
    echo "  expected exit code 3 (ERROR_IO) and 'exceeds maximum size limit'"
    echo "  got exit code: $exit_code"
    echo "  output: $output"
  fi
}

# Test: Input size limit - 1MB + 1 byte should fail
test_max_input_size_one_over() {
  local exit_code=0
  local output
  # Generate 1MB + 1 byte (1048577 bytes) by creating 1MB and appending 1 byte
  output="$({ dd if=/dev/zero bs=1M count=1 2>/dev/null | tr '\0' 'x'; echo -n 'x'; } | NO_COLOR=1 "$BIN" 2>&1)" || exit_code=$?

  # Should trigger size limit error (exit code 3 for ERROR_IO)
  if [[ "$exit_code" -eq 3 ]] && echo "$output" | grep -q "exceeds maximum size limit"; then
    test_passed "Input at 1MB+1 byte rejected with size limit error"
  else
    test_failed "Input at 1MB+1 byte rejected with size limit error"
    echo "  expected exit code 3 (ERROR_IO) and 'exceeds maximum size limit'"
    echo "  got exit code: $exit_code"
    echo "  output: $output"
  fi
}

# Run all tests
echo "Running memory tests..."
echo "======================="

test_max_input_size_at_limit
test_max_input_size_exceeded
test_max_input_size_one_over

# Summary
echo "======================="
TOTAL=$((PASS + FAIL))
echo "Results: $PASS/$TOTAL passed"

if [[ "$FAIL" -gt 0 ]]; then
  echo -e "${RED}FAILED${NC}: $FAIL test(s) failed"
  exit 1
else
  echo -e "${GREEN}SUCCESS${NC}: All tests passed"
  exit 0
fi
