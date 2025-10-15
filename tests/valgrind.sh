#!/usr/bin/env bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BIN_DEBUG="$ROOT/bin/mini-ccstatus-debug"
FIXTURES_DIR="$ROOT/fixtures"

PASS=0
FAIL=0

# Colors for test output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
  echo -e "${RED}error: valgrind is not installed${NC}"
  echo "Install with: sudo apt-get install valgrind (Debian/Ubuntu)"
  echo "          or: sudo dnf install valgrind (Fedora)"
  exit 1
fi

# Build debug version if needed
if [[ ! -x "$BIN_DEBUG" ]]; then
  echo -e "${BLUE}Building debug version...${NC}"
  make -C "$ROOT" debug || {
    echo -e "${RED}error: failed to build debug version${NC}"
    exit 1
  }
fi

test_passed() {
  echo -e "${GREEN}✓${NC} $1"
  PASS=$((PASS + 1))
}

test_failed() {
  echo -e "${RED}✗${NC} $1"
  FAIL=$((FAIL + 1))
}

# Valgrind options for strict leak checking
VALGRIND_OPTS=(
  --leak-check=full
  --show-leak-kinds=all
  --track-origins=yes
  --error-exitcode=99
  --errors-for-leak-kinds=all
  --quiet
)

run_valgrind_test() {
  local test_name="$1"
  local input_source="$2"
  local prog_exit=0
  local output

  # Run valgrind and capture output.
  # Valgrind wraps the program, so we need to distinguish between:
  # - Program failure (exit code from mini-ccstatus) - OK, not our concern
  # - Memory errors (valgrind exits with 99) - FAIL
  # Temporarily disable set -e so program errors don't exit the script
  set +e
  output=$(eval "$input_source" | NO_COLOR=1 valgrind "${VALGRIND_OPTS[@]}" "$BIN_DEBUG" 2>&1)
  prog_exit=$?
  set -e

  # If exit code is 99, valgrind found memory errors
  # Any other exit code means either success (0) or program error (not valgrind error)
  if [[ "$prog_exit" -eq 99 ]]; then
    # Valgrind detected memory issues
    test_failed "$test_name"
    echo -e "${YELLOW}  valgrind detected memory issues:${NC}"
    echo "$output" | sed 's/^/    /'
    return 1
  else
    # No valgrind errors (program may have failed, but that's OK)
    test_passed "$test_name"
    return 0
  fi
}

echo "Running valgrind memory leak tests..."
echo "========================================"
echo ""

# Test all JSON fixtures
if [[ -d "$FIXTURES_DIR" ]]; then
  for fixture in "$FIXTURES_DIR"/*.json; do
    if [[ -f "$fixture" ]]; then
      fixture_name=$(basename "$fixture")
      run_valgrind_test "Fixture: $fixture_name" "cat '$fixture'"
    fi
  done
else
  echo -e "${YELLOW}warning: fixtures directory not found${NC}"
fi

# Test empty input (EOF immediately)
run_valgrind_test "Empty input (immediate EOF)" "echo -n ''"

# Test minimal valid JSON
run_valgrind_test "Minimal valid JSON" "echo '{}'"

# Test invalid JSON (should still not leak)
run_valgrind_test "Invalid JSON (no leaks on error path)" "echo 'not valid json'"

# Test large valid JSON (stress test)
run_valgrind_test "Large valid JSON object" "echo '{\"model\":{\"display_name\":\"test\",\"id\":\"test-id\"},\"cwd\":\"/test\",\"workspace\":{\"project_dir\":\"/test\"},\"version\":\"1.0.0\",\"cost\":{\"total_cost_usd\":0.5,\"total_duration_ms\":1000,\"total_api_duration_ms\":500,\"total_lines_added\":100,\"total_lines_removed\":50},\"exceeds_200k_tokens\":false}'"

# Summary
echo ""
echo "========================================"
TOTAL=$((PASS + FAIL))
echo "Results: $PASS/$TOTAL passed"

if [[ "$FAIL" -gt 0 ]]; then
  echo -e "${RED}FAILED${NC}: $FAIL test(s) failed"
  echo ""
  echo "Valgrind detected memory issues. Please fix memory leaks before committing."
  exit 1
else
  echo -e "${GREEN}SUCCESS${NC}: All tests passed - no memory leaks detected!"
  exit 0
fi
