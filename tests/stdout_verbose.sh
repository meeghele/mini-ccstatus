#!/usr/bin/env bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
BIN="$ROOT/bin/mini-ccstatus"
DOCS="$ROOT/docs"

"$BIN" --verbose <"$DOCS/actual_stdin.json"
