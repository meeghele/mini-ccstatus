#!/usr/bin/env bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

set -euo pipefail

# Move to repository root
cd "$(dirname "${BASH_SOURCE[0]}")/.."

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Building unit tests..."
echo "====================="

# Build test executable
cc -g -O0 -Wall -Wextra -DDEBUG \
   -I. \
   tests/test_token_calculator.c \
   src/token_calculator.c \
   src/safe_conv.c \
   src/json_parser.c \
   lib/cjson/cJSON.c \
   -o tests/test_token_calculator \
   -lm

echo "Running unit tests..."
echo "===================="

# Run tests
if tests/test_token_calculator; then
  echo ""
  echo -e "${GREEN}SUCCESS${NC}: All unit tests passed!"
  exit 0
else
  echo ""
  echo -e "${RED}FAILED${NC}: Some unit tests failed!"
  exit 1
fi