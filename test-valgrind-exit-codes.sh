#!/usr/bin/env bash
# Quick test to verify valgrind exit code behavior

set -u

echo "Testing valgrind exit code behavior..."
echo ""

# Test 1: Valid JSON (program should succeed, no leaks)
echo "Test 1: Valid JSON, no leaks"
echo '{}' | valgrind --error-exitcode=99 --leak-check=full --quiet /home/mtavella/Projects/meeghele/mini-ccstatus/bin/mini-ccstatus-debug 2>&1 > /dev/null
exit_code=$?
echo "  Exit code: $exit_code (expected 0)"
echo ""

# Test 2: Invalid JSON (program should fail with exit 4, but no leaks)
echo "Test 2: Invalid JSON, no leaks expected"
echo 'invalid json' | valgrind --error-exitcode=99 --leak-check=full --quiet /home/mtavella/Projects/meeghele/mini-ccstatus/bin/mini-ccstatus-debug 2>&1 > /dev/null
exit_code=$?
echo "  Exit code: $exit_code (expected 4 if no leaks, 99 if leaks)"
echo ""

echo "If test 2 returns 4, valgrind passes through program exit codes correctly"
echo "If test 2 returns 99, there's a memory leak on error paths"
