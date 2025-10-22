# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

# Benchmark suite for mini-ccstatus and alternative statusline implementations

# Use mise to detect runtime paths if not provided
PYTHON ?= $(shell mise which python3 2>/dev/null || which python3)
NODE ?= $(shell mise which node 2>/dev/null || which node)

BENCH_TIME     := scripts/bench_time.sh
BENCH_CYCLES   := scripts/bench_cycles.sh
BENCH_MEMORY   := scripts/bench_memory.sh
REPORT_SCRIPT  := scripts/generate_report.sh
RESULTS_FILE   := README.md

export PYTHON
export NODE

.PHONY: default
default: time cycles memory

.PHONY: all
all: default

.PHONY: results
results: generate_report

.PHONY: time
time: $(BENCH_TIME)
	@echo "Running time benchmarks..."
	@$(BENCH_TIME) 10

.PHONY: cycles
cycles: $(BENCH_CYCLES)
	@echo "Running CPU cycles benchmarks..."
	@$(BENCH_CYCLES) 10

.PHONY: memory
memory: $(BENCH_MEMORY)
	@echo "Running memory benchmarks..."
	@$(BENCH_MEMORY) 10

.PHONY: generate_report
generate_report: $(REPORT_SCRIPT)
	@echo "Generating benchmark report..."
	@$(REPORT_SCRIPT) 250

.PHONY: clean
clean:
	@echo "Cleaning benchmark artifacts..."
	rm -fv $(RESULTS_FILE)
