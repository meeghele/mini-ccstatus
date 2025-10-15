#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# Time benchmarking for mini-ccstatus and alternatives
# Run ./install.sh first to install dependencies

# Configuration
WARMUP=10
RUNS=250
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCHMARK_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
INPUT_FILE="$SCRIPT_DIR/../../docs/actual_stdin.json"
COMMANDS_FILE="$BENCHMARK_DIR/data/commands.txt"

# Use env vars for interpreters, fallback to mise or which
PYTHON="${PYTHON:-$(mise which python3 2>/dev/null || which python3)}"
NODE="${NODE:-$(mise which node 2>/dev/null || which node)}"

# Check dependencies
if ! command -v hyperfine &> /dev/null; then
    echo "Error: hyperfine not found. Run ./install.sh first."
    exit 1
fi

echo "Time Benchmark Results"
echo "======================"
echo "Config: warmup=${WARMUP}, runs=${RUNS}"
echo ""

# Benchmark function
bench_time() {
    local name="$1"
    local cmd="$2"

    echo "Benchmarking (no shell): $name"
    hyperfine -w "$WARMUP" -m "$RUNS" --prepare 'sync' -N --input "$INPUT_FILE" "$cmd"
    echo ""
}

# Run benchmarks from commands.txt
while IFS='|' read -r name author stack url command type; do
    # Skip comments and empty lines
    [[ "$name" =~ ^#.*$ || -z "$name" ]] && continue

    # Build the full command based on type
    case "$type" in
        python)
            full_cmd="$PYTHON $BENCHMARK_DIR/$command"
            ;;
        node)
            full_cmd="$NODE $BENCHMARK_DIR/$command"
            ;;
        direct)
            full_cmd="$BENCHMARK_DIR/$command"
            ;;
        *)
            echo "Warning: Unknown type '$type' for $name, skipping"
            continue
            ;;
    esac

    # Format the test name for output
    test_name="$name (@$author, $stack, $url)"

    # Run the benchmark
    bench_time "$test_name" "$full_cmd"
done < "$COMMANDS_FILE"

echo "======================"
echo "Time benchmark complete!"
