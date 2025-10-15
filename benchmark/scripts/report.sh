#!/bin/bash
# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>

# Generate benchmark report in Markdown format for README
# This script runs both time and memory benchmarks and formats results

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCHMARK_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_FILE="${1:-$BENCHMARK_DIR/README.md}"
TEMP_DIR=$(mktemp -d)
TIME_OUTPUT="$TEMP_DIR/time_output.txt"
MEMORY_OUTPUT="$TEMP_DIR/memory_output.txt"

cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

echo "Running benchmarks and generating report..."
echo "This will take a few minutes..."
echo ""

# Run time benchmark
echo "Running time benchmarks..."
"$SCRIPT_DIR/bench_time.sh" > "$TIME_OUTPUT" 2>&1

# Run memory benchmark
echo "Running memory benchmarks..."
"$SCRIPT_DIR/bench_memory.sh" > "$MEMORY_OUTPUT" 2>&1

echo ""
echo "Parsing results and generating Markdown..."
echo ""
echo "Debug: Checking for all test names in time output:"
grep "Benchmarking (no shell):" "$TIME_OUTPUT" | wc -l
echo ""
echo "Debug: Checking for all test names in memory output:"
grep "Benchmarking (no shell):" "$MEMORY_OUTPUT" | wc -l
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

# Parse time benchmark results
echo "## Execution Time" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "| Implementation | Author | Stack | Avg | Min | Max | vs Baseline (Avg) |" >> "$OUTPUT_FILE"
echo "|----------------|--------|-------|-----|-----|-----|-------------------|" >> "$OUTPUT_FILE"

# Extract hyperfine results - robust regex-based parsing with baseline calculation
awk '
function to_microseconds(value, unit) {
    if (unit == "µs") return value
    if (unit == "ms") return value * 1000
    if (unit == "s") return value * 1000000
    if (unit == "ns") return value / 1000
    return value
}

function format_ratio(current, baseline) {
    if (baseline == 0) return "-"
    ratio = current / baseline
    if (ratio < 1.01) return "**baseline**"
    return sprintf("**%.1fx**", ratio)
}

/Benchmarking \(no shell\):/ {
    full_impl = $0
    sub(/.*Benchmarking \(no shell\): /, "", full_impl)

    # Parse "name (@author, stack, url)" format
    # Stack may contain commas, so we parse step by step
    if (match(full_impl, /^([^(]+) \(@(.+)\)$/, arr)) {
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
}
/Time \(mean/ {
    # Extract mean using regex: match number followed by unit (s, ms, µs, ns)
    # Pattern: "Time (mean ± σ):     863.2 µs ± 75.3 µs"
    # Matches: "123.4 s" or "123.4 ms" or "123.4 µs" or "123.4 ns"
    if (match($0, /[0-9]+\.[0-9]+ (s|ms|µs|ns)/)) {
        mean = substr($0, RSTART, RLENGTH)
        # Extract numeric value and unit for baseline calculation
        if (match(mean, /([0-9]+\.[0-9]+) (s|ms|µs|ns)/, marr)) {
            mean_value = marr[1]
            mean_unit = marr[2]
            mean_us = to_microseconds(mean_value, mean_unit)
        }
    }
}
/Range \(min/ {
    # Extract min and max using regex
    # Pattern: "Range (min … max):   665.7 µs … 1969.6 µs"
    # First match is min
    if (match($0, /[0-9]+\.[0-9]+ (s|ms|µs|ns)/)) {
        min = substr($0, RSTART, RLENGTH)
        # Continue from after first match to find max
        remainder = substr($0, RSTART + RLENGTH)
        if (match(remainder, /[0-9]+\.[0-9]+ (s|ms|µs|ns)/)) {
            max = substr(remainder, RSTART, RLENGTH)
        }
    }

    if (impl != "" && mean != "" && min != "" && max != "") {
        # Store baseline (mini-ccstatus) value
        if (impl == "mini-ccstatus") {
            baseline_time_us = mean_us
        }

        # Calculate relative performance
        if (baseline_time_us > 0) {
            relative = format_ratio(mean_us, baseline_time_us)
        } else {
            relative = "pending"
        }

        printf "| %s | %s | %s | %s | %s | %s | %s |\n", impl, author, stack, mean, min, max, relative
        impl = ""
        author = ""
        stack = ""
        mean = ""
        min = ""
        max = ""
    }
}
' "$TIME_OUTPUT" >> "$OUTPUT_FILE"

echo "" >> "$OUTPUT_FILE"

# Parse memory benchmark results
echo "## Memory Usage" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "| Implementation | Author | Stack | Avg KB | Avg MB | vs Baseline |" >> "$OUTPUT_FILE"
echo "|----------------|--------|-------|--------|--------|-------------|" >> "$OUTPUT_FILE"

# Extract memory results
awk '
function format_ratio(current, baseline) {
    if (baseline == 0) return "-"
    ratio = current / baseline
    if (ratio < 1.01) return "**baseline**"
    return sprintf("**%.1fx**", ratio)
}

/Benchmarking \(no shell\):/ {
    full_impl = $0
    sub(/.*Benchmarking \(no shell\): /, "", full_impl)

    # Parse "name (@author, stack, url)" format
    # Stack may contain commas, so we parse step by step
    if (match(full_impl, /^([^(]+) \(@(.+)\)$/, arr)) {
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

        # Store baseline (mini-ccstatus) value
        if (impl == "mini-ccstatus") {
            baseline_mem_kb = kb
        }

        # Calculate relative memory usage
        if (baseline_mem_kb > 0) {
            relative = format_ratio(kb, baseline_mem_kb)
        } else {
            relative = "pending"
        }

        printf "| %s | %s | %s | %s | %s | %s |\n", impl, author, stack, kb, mb, relative
    }
}
' "$MEMORY_OUTPUT" >> "$OUTPUT_FILE"

echo "" >> "$OUTPUT_FILE"

# Add benchmark configuration
echo "## Configuration" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "- **Time Benchmark:** $(grep -oP 'warmup=\d+, runs=\d+' "$TIME_OUTPUT")" >> "$OUTPUT_FILE"
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
