# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

# Benchmark suite for mini-ccstatus and alternative statusline implementations

# Use mise to detect runtime paths if not provided
PYTHON ?= $(shell mise which python3 2>/dev/null || which python3)
NODE ?= $(shell mise which node 2>/dev/null || which node)

INSTALL_SCRIPT := scripts/install.sh
BENCH_TIME     := scripts/bench_time.sh
BENCH_MEMORY   := scripts/bench_memory.sh
REPORT_SCRIPT  := scripts/report.sh
RESULTS_FILE   := README.md

export PYTHON
export NODE

.PHONY: default
default: time memory

.PHONY: all
all: clean install default

.PHONY: results
results: clean install report

.PHONY: install
install: $(INSTALL_SCRIPT)
	@echo "Installing benchmark dependencies..."
	@$(INSTALL_SCRIPT)

.PHONY: time
time: $(BENCH_TIME)
	@echo "Running time benchmarks..."
	@$(BENCH_TIME)

.PHONY: memory
memory: $(BENCH_MEMORY)
	@echo "Running memory benchmarks..."
	@$(BENCH_MEMORY)

.PHONY: report
report: $(REPORT_SCRIPT)
	@echo "Generating benchmark report..."
	@$(REPORT_SCRIPT)

.PHONY: clean
clean:
	@echo "Cleaning benchmark artifacts..."
	rm -rfv node_modules package-lock.json $(RESULTS_FILE)