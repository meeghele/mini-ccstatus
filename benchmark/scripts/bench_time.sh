#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# Time benchmarking for mini-ccstatus and alternatives
# Run ./install.sh first to install dependencies

# Configuration
WARMUP=10
RUNS=${1:-250}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCHMARK_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_DIR="$(cd "$BENCHMARK_DIR/.." && pwd)"
INPUT_FILE="${INPUT_FILE:-$PROJECT_DIR/docs/actual_stdin.json}"
COMMANDS_FILE="$BENCHMARK_DIR/data/commands.txt"

# Cleanup function
cleanup() {
    rm -f /tmp/bench_wrapper_$$_*.sh
}
trap cleanup EXIT

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
echo "Input: ${INPUT_FILE}"
echo ""

# Benchmark function
bench_time() {
    local name="$1"
    local cmd="$2"
    local bench_index="$3"

    echo "Benchmarking (no shell): $name"

    # Export JSON if JSON_OUTPUT_DIR is set
    if [[ -n "$JSON_OUTPUT_DIR" ]]; then
        local json_file="$JSON_OUTPUT_DIR/bench_${bench_index}.json"
        hyperfine -w "$WARMUP" -m "$RUNS" --prepare 'sync' -N --input "$INPUT_FILE" \
            --export-json "$json_file" "$cmd"
    else
        hyperfine -w "$WARMUP" -m "$RUNS" --prepare 'sync' -N --input "$INPUT_FILE" "$cmd"
    fi
    echo ""
}

# Run benchmarks from commands.txt
bench_index=0
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
        system)
            full_cmd="$command"
            ;;
        *)
            echo "Warning: Unknown type '$type' for $name, skipping"
            continue
            ;;
    esac

    # Format the test name for output
    test_name="$name (@$author, $stack, $url)"

    # Run the benchmark
    bench_time "$test_name" "$full_cmd" "$bench_index"

    # Increment index for valid benchmarks
    bench_index=$((bench_index + 1))
done < "$COMMANDS_FILE"

echo "======================"
echo "Time benchmark complete!"
