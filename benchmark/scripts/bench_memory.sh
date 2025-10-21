#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# Memory benchmarking for mini-ccstatus and alternatives
# Run ./install.sh first to install dependencies

# Configuration
RUNS=${1:-250}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCHMARK_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_DIR="$(cd "$BENCHMARK_DIR/.." && pwd)"
INPUT_FILE="${INPUT_FILE:-$PROJECT_DIR/docs/actual_stdin.json}"
COMMANDS_FILE="$BENCHMARK_DIR/data/commands.txt"

# Cleanup function
cleanup() {
    :
}
trap cleanup EXIT

# Use env vars for interpreters, fallback to mise or which
PYTHON="${PYTHON:-$(mise which python3 2>/dev/null || which python3)}"
NODE="${NODE:-$(mise which node 2>/dev/null || which node)}"

# Check dependencies
if ! command -v /usr/bin/time &> /dev/null; then
    echo "Error: GNU time not found. Run ./install.sh first."
    exit 1
fi

echo "Memory Benchmark Results"
echo "========================"
echo "Config: runs=${RUNS}"
echo "Input: ${INPUT_FILE}"
echo ""

# Benchmark function - executes command directly without shell wrapper
# Usage: bench_memory "name" cmd_array...
bench_memory() {
    local name="$1"
    shift
    local cmd=("$@")
    local total_maxrss=0

    echo "Benchmarking (no shell): $name"

    for i in $(seq 1 $RUNS); do
        # Execute the command directly so the measured RSS excludes shell overhead
        output=$(/usr/bin/time -f "%M" "${cmd[@]}" < "$INPUT_FILE" 2>&1 >/dev/null | tail -1)
        total_maxrss=$((total_maxrss + output))
    done

    local avg_kb=$((total_maxrss / RUNS))
    local avg_mb=$((avg_kb / 1024))

    echo "  Average Max RSS: ${avg_kb} KB (${avg_mb} MB) over ${RUNS} runs"
    echo ""
}

# Run benchmarks from commands.txt
while IFS='|' read -r name author stack url command type; do
    # Skip comments and empty lines
    [[ "$name" =~ ^#.*$ || -z "$name" ]] && continue

    # Build the full command based on type
    case "$type" in
        python)
            cmd=("$PYTHON" "$BENCHMARK_DIR/$command")
            ;;
        node)
            cmd=("$NODE" "$BENCHMARK_DIR/$command")
            ;;
        direct)
            cmd=("$BENCHMARK_DIR/$command")
            ;;
        system)
            read -ra cmd_parts <<< "$command"
            cmd=("${cmd_parts[@]}")
            ;;
        *)
            echo "Warning: Unknown type '$type' for $name, skipping"
            continue
            ;;
    esac

    # Format the test name for output
    test_name="$name (@$author, $stack, $url)"

    # Run the benchmark
    bench_memory "$test_name" "${cmd[@]}"
done < "$COMMANDS_FILE"

echo "========================"
echo "Memory benchmark complete!"
