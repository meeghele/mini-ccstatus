#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# Generate benchmark report in Markdown format for README
# This script runs both time and memory benchmarks and formats results

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCHMARK_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
RUNS="${1:-250}"
OUTPUT_FILE="${2:-$BENCHMARK_DIR/REPORT.md}"
TEMP_DIR=$(mktemp -d)
TIME_OUTPUT="$TEMP_DIR/time_output.txt"
CYCLES_OUTPUT="$TEMP_DIR/cycles_output.txt"
MEMORY_OUTPUT="$TEMP_DIR/memory_output.txt"
JSON_OUTPUT_DIR="$TEMP_DIR/json"
mkdir -p "$JSON_OUTPUT_DIR"

cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

echo "Running benchmarks and generating report..."
echo "This will take a few minutes..."
echo ""

# Run time benchmark
echo "Running time benchmarks..."
JSON_OUTPUT_DIR="$JSON_OUTPUT_DIR" "$SCRIPT_DIR/bench_time.sh" "$RUNS" > "$TIME_OUTPUT" 2>&1

# Run cycles benchmark
echo "Running CPU cycles benchmarks..."
JSON_OUTPUT_DIR="$JSON_OUTPUT_DIR" "$SCRIPT_DIR/bench_cycles.sh" "$RUNS" > "$CYCLES_OUTPUT" 2>&1

# Run memory benchmark
echo "Running memory benchmarks..."
"$SCRIPT_DIR/bench_memory.sh" "$RUNS" > "$MEMORY_OUTPUT" 2>&1

echo ""
echo "Parsing results and generating Markdown..."
echo ""
echo "Debug: Checking for all test names in time output:"
grep "Benchmarking (no shell):" "$TIME_OUTPUT" | wc -l
echo ""
echo "Debug: Checking for all test names in memory output:"
grep "Benchmarking (no shell):" "$MEMORY_OUTPUT" | wc -l
echo ""
echo "Debug: Checking for all test names in cycles output:"
grep "Benchmarking (perf stat):" "$CYCLES_OUTPUT" | wc -l
echo ""

# Generate the markdown report
cat "$BENCHMARK_DIR/data/heading.md" > "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Add timestamp
echo "**Last Updated:** $(date '+%Y-%m-%d %H:%M:%S %Z')" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Add projects list with URLs
echo "## Implementations" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Read from commands.txt and generate list
while IFS='|' read -r name author stack url command type; do
    # Skip comments and empty lines
    [[ "$name" =~ ^#.*$ || -z "$name" ]] && continue

    # Format as markdown list item with link to project and author in parentheses
    if [[ "$url" != "-" && -n "$url" ]]; then
        echo "- [**$name**](https://$url) (@$author) - $stack" >> "$OUTPUT_FILE"
    else
        echo "- **$name** (@$author) - $stack" >> "$OUTPUT_FILE"
    fi
done < "$BENCHMARK_DIR/data/commands.txt"

echo "" >> "$OUTPUT_FILE"

# Add note about third-party tools
echo "**Note:** Unlike the reference implementations that only process JSON from stdin, \`ccusage\` and \`ccstatusline\` also read configuration files from \`\$HOME/.claude\` and may make API calls. The performance measurements for these tools reflect their full behavior and may vary depending on your system configuration, network latency, and Claude API response times." >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Parse time benchmark results
echo "## Execution Time" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |" >> "$OUTPUT_FILE"
echo "|----------------|--------|-------|-----|-----|-----|-------------------|" >> "$OUTPUT_FILE"

# Extract hyperfine results - parse JSON files with jq
# First pass: collect all results and find baseline
bench_index=0
declare -a results
baseline_time_s=""

while IFS='|' read -r name author stack url command type; do
    # Skip comments and empty lines
    [[ "$name" =~ ^#.*$ || -z "$name" ]] && continue

    json_file="$JSON_OUTPUT_DIR/bench_${bench_index}.json"

    if [[ ! -f "$json_file" ]]; then
        echo "Warning: Missing JSON file for $name" >&2
        bench_index=$((bench_index + 1))
        continue
    fi

    # Extract timing data from JSON (hyperfine stores times in seconds)
    mean_s=$(jq -r '.results[0].mean' "$json_file")
    min_s=$(jq -r '.results[0].min' "$json_file")
    max_s=$(jq -r '.results[0].max' "$json_file")

    # Store result for second pass
    results+=("$name|$author|$stack|$url|$mean_s|$min_s|$max_s")

    bench_index=$((bench_index + 1))
done < "$BENCHMARK_DIR/data/commands.txt"

# Second pass: format output with baseline comparison
first_time_entry=true
for result in "${results[@]}"; do
    IFS='|' read -r name author stack url mean_s min_s max_s <<< "$result"

    # Format times with appropriate units (hyperfine outputs in seconds as float)
    mean_formatted=$(awk -v val="$mean_s" 'BEGIN {
        if (val >= 1) printf "%.1f s", val
        else if (val >= 0.001) printf "%.1f ms", val * 1000
        else printf "%.1f µs", val * 1000000
    }')
    min_formatted=$(awk -v val="$min_s" 'BEGIN {
        if (val >= 1) printf "%.1f s", val
        else if (val >= 0.001) printf "%.1f ms", val * 1000
        else printf "%.1f µs", val * 1000000
    }')
    max_formatted=$(awk -v val="$max_s" 'BEGIN {
        if (val >= 1) printf "%.1f s", val
        else if (val >= 0.001) printf "%.1f ms", val * 1000
        else printf "%.1f µs", val * 1000000
    }')

    # Calculate ratio vs baseline
    if $first_time_entry; then
        baseline_time_s="$mean_s"
        relative="**baseline**"
        first_time_entry=false
    elif [[ -n "$baseline_time_s" && "$baseline_time_s" != "0" ]]; then
        relative=$(awk -v current="$mean_s" -v baseline="$baseline_time_s" 'BEGIN {
            if (baseline == 0) {
                print "-"
                exit
            }
            ratio = current / baseline
            printf "**%.1fx**", ratio
        }')
    else
        relative="-"
    fi

    # Format name as link if URL exists
    if [[ "$url" != "-" && -n "$url" ]]; then
        name_formatted="[$name](https://$url)"
    else
        name_formatted="$name"
    fi

    echo "| $name_formatted | $author | $stack | $mean_formatted | $min_formatted | $max_formatted | $relative |" >> "$OUTPUT_FILE"
done

echo "" >> "$OUTPUT_FILE"

# Parse CPU cycles benchmark results
echo "## CPU Cycles" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "| Implementation | Author | Stack | Cycles | Instructions | IPC | Cache Miss Rate | vs Baseline (Cycles) |" >> "$OUTPUT_FILE"
echo "|----------------|--------|-------|--------|--------------|-----|-----------------|----------------------|" >> "$OUTPUT_FILE"

# Extract cycles results from JSON files
bench_index=0
declare -a cycles_results
baseline_cycles=""

while IFS='|' read -r name author stack url command type; do
    # Skip comments and empty lines
    [[ "$name" =~ ^#.*$ || -z "$name" ]] && continue

    json_file="$JSON_OUTPUT_DIR/cycles_${bench_index}.json"

    if [[ ! -f "$json_file" ]]; then
        echo "Warning: Missing cycles JSON file for $name" >&2
        bench_index=$((bench_index + 1))
        continue
    fi

    # Extract cycles data from JSON
    cycles=$(jq -r '.cycles' "$json_file")
    instructions=$(jq -r '.instructions' "$json_file")
    ipc=$(jq -r '.ipc' "$json_file")
    cache_miss_rate=$(jq -r '.cache_miss_rate' "$json_file")

    # Store result for second pass
    cycles_results+=("$name|$author|$stack|$url|$cycles|$instructions|$ipc|$cache_miss_rate")

    bench_index=$((bench_index + 1))
done < "$BENCHMARK_DIR/data/commands.txt"

# Second pass: format output with baseline comparison
first_cycle_entry=true
for result in "${cycles_results[@]}"; do
    IFS='|' read -r name author stack url cycles instructions ipc cache_miss_rate <<< "$result"

    # Format cycles with thousands separator
    cycles_formatted=$(printf "%'d" "$cycles" 2>/dev/null || echo "$cycles")
    instructions_formatted=$(printf "%'d" "$instructions" 2>/dev/null || echo "$instructions")

    # Calculate ratio vs baseline
    if $first_cycle_entry; then
        baseline_cycles="$cycles"
        relative="**baseline**"
        first_cycle_entry=false
    elif [[ -n "$baseline_cycles" && "$baseline_cycles" != "0" ]]; then
        relative=$(awk -v current="$cycles" -v baseline="$baseline_cycles" 'BEGIN {
            if (baseline == 0) {
                print "-"
                exit
            }
            ratio = current / baseline
            printf "**%.1fx**", ratio
        }')
    else
        relative="-"
    fi

    # Format name as link if URL exists
    if [[ "$url" != "-" && -n "$url" ]]; then
        name_formatted="[$name](https://$url)"
    else
        name_formatted="$name"
    fi

    echo "| $name_formatted | $author | $stack | $cycles_formatted | $instructions_formatted | $ipc | $cache_miss_rate | $relative |" >> "$OUTPUT_FILE"
done

echo "" >> "$OUTPUT_FILE"

# Parse memory benchmark results
echo "## Memory Usage" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |" >> "$OUTPUT_FILE"
echo "|----------------|--------|-------|--------|--------|-------------|" >> "$OUTPUT_FILE"

# Extract memory results
awk '
BEGIN {
    row_index = 0
    baseline_mem_kb = 0
}
function format_ratio(current, baseline) {
    if (baseline == 0) return "-"
    ratio = current / baseline
    return sprintf("**%.1fx**", ratio)
}

/Benchmarking \(no shell\):/ {
    full_impl = $0
    sub(/.*Benchmarking \(no shell\): /, "", full_impl)

    # Parse "name (@author, stack, url)" format
    # Stack may contain commas, so we parse step by step
    # Match everything up to the last " (@" to handle names with parentheses
    if (match(full_impl, /^(.+) \(@(.+)\)$/, arr)) {
        impl = arr[1]
        gsub(/^ +| +$/, "", impl)  # trim whitespace

        # Parse the parentheses content: "author, stack..., url"
        paren_content = arr[2]

        # Extract author (first comma-separated part)
        if (match(paren_content, /^([^,]+), (.+)$/, parts)) {
            author = parts[1]
            gsub(/^ +| +$/, "", author)

            remainder = parts[2]

            # Extract URL (last comma-separated part)
            # Match everything up to the last comma, then the URL
            if (match(remainder, /^(.*), ([^,]+)$/, urlparts)) {
                stack = urlparts[1]
                gsub(/^ +| +$/, "", stack)
                url = urlparts[2]
                gsub(/^ +| +$/, "", url)
            } else {
                # No URL, only stack
                stack = remainder
                gsub(/^ +| +$/, "", stack)
                url = ""
            }
        } else {
            author = "-"
            stack = "-"
            url = ""
        }
    } else {
        # Fallback if format doesn'\''t match
        impl = full_impl
        author = "-"
        stack = "-"
        url = ""
    }

    getline
    if ($0 ~ /Average Max RSS:/) {
        # Extract KB value (e.g., "3455 KB")
        kb = $4
        # Extract MB value from parentheses (e.g., "(3 MB)")
        mb = $6
        gsub(/\(/, "", mb)
        gsub(/\)/, "", mb)

        # Track baseline row (first entry)
        row_index++
        if (row_index == 1) {
            baseline_mem_kb = kb
            relative = "**baseline**"
        } else if (baseline_mem_kb > 0) {
            relative = format_ratio(kb, baseline_mem_kb)
        } else {
            relative = "pending"
        }

        # Format impl as link if URL exists
        if (url != "" && url != "-") {
            impl_formatted = sprintf("[%s](https://%s)", impl, url)
        } else {
            impl_formatted = sprintf("%s", impl)
        }

        printf "| %s | %s | %s | %s | %s | %s |\n", impl_formatted, author, stack, kb, mb, relative
    }
}
' "$MEMORY_OUTPUT" >> "$OUTPUT_FILE"

echo "" >> "$OUTPUT_FILE"

# Add benchmark configuration
echo "## Configuration" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "- **Time Benchmark:** $(grep -oP 'warmup=\d+, runs=\d+' "$TIME_OUTPUT")" >> "$OUTPUT_FILE"
echo "- **CPU Cycles Benchmark:** $(grep -oP 'runs=\d+' "$CYCLES_OUTPUT")" >> "$OUTPUT_FILE"
echo "- **Memory Benchmark:** $(grep -oP 'runs=\d+' "$MEMORY_OUTPUT")" >> "$OUTPUT_FILE"
echo "- **System:** $(uname -s) $(uname -r)" >> "$OUTPUT_FILE"
echo "- **CPU:** $(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Add runtime versions
echo "### Runtime Versions" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# C compiler
if command -v cc &> /dev/null; then
    CC_VERSION=$(cc --version 2>/dev/null | head -1 || echo "unknown")
    echo "- **C Compiler:** $CC_VERSION" >> "$OUTPUT_FILE"
fi

# Python
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2)
    echo "- **Python:** $PYTHON_VERSION" >> "$OUTPUT_FILE"
fi

# Node.js
if command -v node &> /dev/null; then
    NODE_VERSION=$(node --version | sed 's/^v//')
    echo "- **Node.js:** $NODE_VERSION" >> "$OUTPUT_FILE"
fi

# Bash
if command -v bash &> /dev/null; then
    BASH_VERSION=$(bash --version | head -1 | grep -oP '\d+\.\d+\.\d+')
    echo "- **Bash:** $BASH_VERSION" >> "$OUTPUT_FILE"
fi

# jq
if command -v jq &> /dev/null; then
    JQ_VERSION=$(jq --version | sed 's/jq-//')
    echo "- **jq:** $JQ_VERSION" >> "$OUTPUT_FILE"
fi

echo "" >> "$OUTPUT_FILE"

# Add notes
cat "$BENCHMARK_DIR/data/notes.md" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

echo ""
echo "✓ Report generated: $OUTPUT_FILE"
echo ""
echo "Preview:"
echo "========================================"
head -30 "$OUTPUT_FILE"
echo "..."
echo "========================================"
