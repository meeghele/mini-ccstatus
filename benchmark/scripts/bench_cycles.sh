#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# CPU cycles benchmarking for mini-ccstatus and alternatives using perf stat
# Run ./install.sh first to install dependencies

# Configuration
RUNS=${1:-250}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCHMARK_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_DIR="$(cd "$BENCHMARK_DIR/.." && pwd)"
INPUT_FILE="${INPUT_FILE:-$PROJECT_DIR/docs/actual_stdin.json}"
COMMANDS_FILE="$BENCHMARK_DIR/data/commands.txt"
TEMP_DIR=$(mktemp -d)

# Cleanup function
cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

# Use env vars for interpreters, fallback to mise or which
PYTHON="${PYTHON:-$(mise which python3 2>/dev/null || which python3)}"
NODE="${NODE:-$(mise which node 2>/dev/null || which node)}"

# Check dependencies
if ! command -v perf &> /dev/null; then
    echo "Error: perf not found. Install with: sudo dnf install perf"
    exit 1
fi

echo "CPU Cycles Benchmark Results"
echo "============================"
echo "Config: runs=${RUNS}"
echo "Input: ${INPUT_FILE}"
echo ""

# Benchmark function - uses perf stat to measure CPU cycles
# Usage: bench_cycles "name" cmd_array...
bench_cycles() {
    local name="$1"
    shift
    local cmd=("$@")
    local perf_output="$TEMP_DIR/perf_$$_$(date +%s%N).csv"
    local wrapper_script="$TEMP_DIR/wrapper_$$_$(date +%s%N).sh"

    echo "Benchmarking (perf stat): $name"

    # Create a wrapper script to handle stdin redirection properly with perf
    cat > "$wrapper_script" <<'WRAPPER_EOF'
#!/bin/bash
WRAPPER_EOF
    echo "exec" "${cmd[@]@Q}" "< \"\$1\"" >> "$wrapper_script"
    chmod +x "$wrapper_script"

    # Run perf stat with CSV output
    # -r: repeat N times
    # -x,: use comma as field separator for CSV
    # Events: cycles, instructions, cache-misses, cache-references
    perf stat -r "$RUNS" -x, \
        -e cycles,instructions,cache-misses,cache-references \
        -o "$perf_output" \
        "$wrapper_script" "$INPUT_FILE" >/dev/null 2>&1

    local exit_code=$?
    rm -f "$wrapper_script"

    if [[ $exit_code -ne 0 ]]; then
        echo "  Error: perf stat failed for $name"
        echo ""
        return 1
    fi

    # Parse perf stat CSV output
    # Format: value,unit,event,variance,runtime,...
    # Use the final summary line for each event to avoid skew from individual runs
    local cycles=$(grep -E "cycles" "$perf_output" | grep -v "<not counted>" | tail -1 | cut -d, -f1 | tr -d ' ')
    local instructions=$(grep -E "instructions" "$perf_output" | grep -v "<not counted>" | tail -1 | cut -d, -f1 | tr -d ' ')
    local cache_misses=$(grep -E "cache-misses" "$perf_output" | grep -v "<not counted>" | tail -1 | cut -d, -f1 | tr -d ' ')
    local cache_refs=$(grep -E "cache-references" "$perf_output" | grep -v "<not counted>" | tail -1 | cut -d, -f1 | tr -d ' ')

    # Calculate IPC (instructions per cycle)
    local ipc=""
    if [[ -n "$cycles" && -n "$instructions" && "$cycles" != "0" ]]; then
        ipc=$(awk -v inst="$instructions" -v cyc="$cycles" 'BEGIN {printf "%.2f", inst/cyc}')
    fi

    # Calculate cache miss rate
    local cache_miss_rate=""
    if [[ -n "$cache_misses" && -n "$cache_refs" && "$cache_refs" != "0" ]]; then
        cache_miss_rate=$(awk -v misses="$cache_misses" -v refs="$cache_refs" 'BEGIN {printf "%.2f%%", (misses/refs)*100}')
    fi

    # Format output
    echo "  Cycles: ${cycles:-N/A}"
    echo "  Instructions: ${instructions:-N/A}"
    echo "  IPC: ${ipc:-N/A}"
    echo "  Cache Misses: ${cache_misses:-N/A}"
    echo "  Cache Miss Rate: ${cache_miss_rate:-N/A}"
    echo ""

    # Export JSON if JSON_OUTPUT_DIR is set
    if [[ -n "$JSON_OUTPUT_DIR" ]]; then
        local json_file="$JSON_OUTPUT_DIR/cycles_${bench_index}.json"
        cat > "$json_file" <<EOF
{
  "name": "$name",
  "cycles": "${cycles:-0}",
  "instructions": "${instructions:-0}",
  "ipc": "${ipc:-0}",
  "cache_misses": "${cache_misses:-0}",
  "cache_references": "${cache_refs:-0}",
  "cache_miss_rate": "${cache_miss_rate:-0}"
}
EOF
    fi
}

# Run benchmarks from commands.txt
bench_index=0
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
    bench_cycles "$test_name" "${cmd[@]}"

    # Increment index for valid benchmarks
    bench_index=$((bench_index + 1))
done < "$COMMANDS_FILE"

echo "============================"
echo "CPU cycles benchmark complete!"
