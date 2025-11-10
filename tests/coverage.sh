#!/usr/bin/env bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BIN="$ROOT/bin/mini-ccstatus"
FIXTURES="$ROOT/fixtures"

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
  echo -e "${GREEN}PASS${NC} $1"
  PASS=$((PASS + 1))
}

test_failed() {
  echo -e "${RED}FAIL${NC} $1"
  FAIL=$((FAIL + 1))
}

test_skipped() {
  echo -e "${YELLOW}SKIP${NC} $1"
}

# Test: Basic single status line (NO_COLOR)
test_basic_status() {
  local exit_code=0
  local actual
  actual="$(NO_COLOR=1 "$BIN" <"$FIXTURES/status.json")" || exit_code=$?

  # Check for <200k badge after cost
  if [[ "$exit_code" -eq 0 ]] && echo "$actual" | grep -q "Opus" && echo "$actual" | grep -q "<200k"; then
    test_passed "Basic status line (NO_COLOR)"
  else
    test_failed "Basic status line (NO_COLOR)"
    echo "  expected badge <200k after cost"
    echo "  actual:   $actual"
    echo "  exit code: $exit_code"
  fi
}


# Test: Pretty-printed multi-line JSON (not supported - line-based parser)
test_multi_pretty() {
  local exit_code=0
  NO_COLOR=1 "$BIN" <"$FIXTURES/multi_pretty.json" >/dev/null 2>&1 || exit_code=$?

  # Pretty-printed JSON should produce ERROR_JSON (4) - parser is line-based
  if [[ "$exit_code" -eq 4 ]]; then
    test_passed "Pretty-printed JSON rejected (line-based parser)"
  else
    test_failed "Pretty-printed JSON rejected (line-based parser)"
    echo "  expected exit code 4 (ERROR_JSON), got $exit_code"
  fi
}

# Test: Edge cases (paths with trailing slashes, empty paths, etc.)
test_edge_cases() {
  local outputs=()
  local exit_code=0

  for i in 01 02 03 04 05; do
    local out
    out="$(NO_COLOR=1 "$BIN" <"$FIXTURES/edge_case_$i.json")" || exit_code=$?
    if [[ "$exit_code" -ne 0 ]]; then
      test_failed "Edge cases (paths, trailing slashes)"
      echo "  non-zero exit while processing edge_case_$i.json: $exit_code"
      return
    fi
    outputs+=("$out")
  done

  if [[ "${#outputs[@]}" -ne 5 ]]; then
    test_failed "Edge cases (paths, trailing slashes)"
    echo "  processed entries: ${#outputs[@]} (expected 5)"
    printf '  outputs:\n'
    printf '    %s\n' "${outputs[@]}"
    return
  fi

  local expected=(Test1 Test2 Test3 Test4 Test5)
  local model
  for model in "${expected[@]}"; do
    local found=0
    local out
    for out in "${outputs[@]}"; do
      if [[ "$out" == *"$model"* ]]; then
        found=1
        break
      fi
    done
    if [[ "$found" -eq 0 ]]; then
      test_failed "Edge cases (paths, trailing slashes)"
      echo "  missing model: $model"
      printf '  outputs:\n'
      printf '    %s\n' "${outputs[@]}"
      return
    fi
  done

  test_passed "Edge cases (paths, trailing slashes)"
}

# Test: Invalid JSON handling
test_invalid_json() {
  local exit_code=0
  NO_COLOR=1 "$BIN" <"$FIXTURES/invalid_json.json" >/dev/null 2>&1 || exit_code=$?

  # Should return ERROR_JSON (4)
  if [[ "$exit_code" -eq 4 ]]; then
    test_passed "Invalid JSON error handling"
  else
    test_failed "Invalid JSON error handling"
    echo "  expected exit code 4 (ERROR_JSON), got $exit_code"
  fi
}


# Test: Missing required fields (should use defaults like "?")
test_missing_fields() {
  local all_passed=true

  for i in 01 02 03 04 05; do
    local exit_code=0
    local output
    output="$(NO_COLOR=1 "$BIN" <"$FIXTURES/missing_fields_$i.json" 2>&1)" || exit_code=$?

    # Should still process successfully (exit 0) with default values
    if [[ "$exit_code" -ne 0 ]] || ! echo "$output" | grep -q "?"; then
      all_passed=false
      echo "  missing_fields_$i.json failed:"
      echo "    expected exit code 0 and '?' for missing fields"
      echo "    got exit code: $exit_code"
      echo "    output: $output"
    fi
  done

  if $all_passed; then
    test_passed "Missing fields handled with defaults"
  else
    test_failed "Missing fields handled with defaults"
  fi
}

# Test: Color output (colors enabled by default)
test_color_output() {
  local output
  output="$("$BIN" <"$FIXTURES/status.json")"

  # Should contain ANSI escape codes
  if echo "$output" | grep -q $'\033'; then
    test_passed "Color output (default mode)"
  else
    test_failed "Color output (default mode)"
    echo "  expected ANSI color codes"
  fi
}

# Test: VERBOSE mode (compact format)
test_verbose_compact() {
  local output
  output="$(NO_COLOR=1 "$BIN" <"$FIXTURES/status.json")"

  # Compact mode should NOT contain "Model:" prefix
  if ! echo "$output" | grep -q "Model:"; then
    test_passed "Compact format (no VERBOSE)"
  else
    test_failed "Compact format (no VERBOSE)"
    echo "  should not contain 'Model:' prefix"
  fi
}

# Test: VERBOSE mode (verbose format)
test_verbose_extended() {
  local output
  output="$(NO_COLOR=1 "$BIN" --verbose <"$FIXTURES/status.json")"

  # Verbose mode SHOULD contain "Model:" prefix
  if echo "$output" | grep -q "Model:"; then
    test_passed "Verbose format (--verbose)"
  else
    test_failed "Verbose format (--verbose)"
    echo "  expected 'Model:' prefix"
  fi
}


# Test: Different cwd vs project_dir displays both
test_different_dirs() {
  local output
  output="$(NO_COLOR=1 "$BIN" <"$FIXTURES/edge_case_02.json")"

  if [[ -n "$output" ]] && echo "$output" | grep -q "root" && echo "$output" | grep -q "user"; then
    test_passed "Different cwd/project_dir displays both"
  else
    test_failed "Different cwd/project_dir displays both"
    echo "  output: $output"
  fi
}

# Test: Same cwd and project_dir shows only one
test_same_dirs() {
  local line1
  line1="$(NO_COLOR=1 "$BIN" <"$FIXTURES/edge_case_01.json")"

  # Should only show directory once (expect format: "... | / | $...")
  # NOT: "... | / | / | $..."
  # Count the number of " | " separators before the cost field
  local dir_section
  dir_section="$(echo "$line1" | sed 's/\$.*$//')"
  local pipe_count
  pipe_count="$(echo "$dir_section" | grep -o ' | ' | wc -l)"

  # Should have 3 pipes: model | version | dir |
  if [[ "$pipe_count" -eq 3 ]]; then
    test_passed "Same cwd/project_dir shows once"
  else
    test_failed "Same cwd/project_dir shows once"
    echo "  line: $line1"
    echo "  pipes before cost: $pipe_count (expected 3)"
  fi
}

# Test: Empty stdin (should exit cleanly)
test_empty_input() {
  local exit_code=0
  echo -n "" | "$BIN" >/dev/null 2>&1 || exit_code=$?

  if [[ "$exit_code" -eq 0 ]]; then
    test_passed "Empty stdin handled gracefully"
  else
    test_failed "Empty stdin handled gracefully"
    echo "  exit code: $exit_code"
  fi
}

# Test: EOF after valid line
test_eof_handling() {
  local exit_code=0
  local output
  output="$(NO_COLOR=1 "$BIN" <"$FIXTURES/status.json")" || exit_code=$?

  if [[ "$exit_code" -eq 0 ]] && [[ -n "$output" ]]; then
    test_passed "EOF after valid line"
  else
    test_failed "EOF after valid line"
    echo "  exit code: $exit_code"
  fi
}

# Test: exceeds_200k_tokens badge display (>200k)
test_exceeds_200k_badge() {
  local exit_code=0
  local output
  output="$(NO_COLOR=1 "$BIN" <"$FIXTURES/exceeds_200k.json")" || exit_code=$?

  # Should show >200k badge after cost
  if [[ "$exit_code" -eq 0 ]] && echo "$output" | grep -q ">200k"; then
    test_passed "exceeds_200k_tokens badge display (>200k)"
  else
    test_failed "exceeds_200k_tokens badge display (>200k)"
    echo "  expected >200k badge after cost in output"
    echo "  output: $output"
    echo "  exit code: $exit_code"
  fi
}


# Run all tests
echo "Running test suite..."
echo "===================="

test_basic_status
test_multi_pretty
test_edge_cases
test_invalid_json
test_missing_fields
test_color_output
test_verbose_compact
test_verbose_extended
test_different_dirs
test_same_dirs
test_empty_input
test_eof_handling
test_exceeds_200k_badge

# Summary
echo "===================="
TOTAL=$((PASS + FAIL))
echo "Results: $PASS/$TOTAL passed"

if [[ "$FAIL" -gt 0 ]]; then
  echo -e "${RED}FAILED${NC}: $FAIL test(s) failed"
  exit 1
else
  echo -e "${GREEN}SUCCESS${NC}: All tests passed"
  exit 0
fi
