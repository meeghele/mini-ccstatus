# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

# Colors are enabled by default; set NO_COLOR in the environment to disable.
CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -O3 -pipe -march=native -flto -DNDEBUG
# Type-safety warning flags (strict mode)
WARNFLAGS ?= -Wall -Wextra -Wpedantic \
             -Wconversion -Wsign-conversion \
             -Wcast-qual -Wformat=2 \
             -Wstrict-overflow=5 -Wwrite-strings -Wundef \
             -Wshadow -Wpointer-arith \
             -Wcast-align -Wstrict-prototypes
LDFLAGS ?= -lm

TARGET := mini-ccstatus
OBJ_DIR := obj
BIN_DIR := bin
SRC_DIR := src
LIB_DIR := lib
TST_DIR := tests
LOG_DIR := log

# Source and object files
SOURCES := main.c \
           $(SRC_DIR)/cache.c \
           $(SRC_DIR)/cli_parser.c \
           $(SRC_DIR)/json_parser.c \
           $(SRC_DIR)/token_calculator.c \
           $(SRC_DIR)/display.c \
           $(SRC_DIR)/safe_conv.c \
           $(LIB_DIR)/cjson/cJSON.c

# Release build configuration
OBJ_DIR_RELEASE := $(OBJ_DIR)/release
OBJECTS := $(addprefix $(OBJ_DIR_RELEASE)/, $(patsubst %.c,%.o,$(notdir $(SOURCES))))

# Common compilation settings
COMPILE_FLAGS := $(CPPFLAGS) -I. -I$(LIB_DIR)
COMMON_DEPS := $(SRC_DIR)/*.h $(LIB_DIR)/cjson/cJSON.h

# Debug build configuration (for valgrind and debugging)
CFLAGS_DEBUG_BASE := -g -O0 $(WARNFLAGS)
CFLAGS_DEBUG  := $(CFLAGS_DEBUG_BASE) -fanalyzer
TARGET_DEBUG  := mini-ccstatus-debug
OBJ_DIR_DEBUG := $(OBJ_DIR)/debug
OBJECTS_DEBUG := $(addprefix $(OBJ_DIR_DEBUG)/, $(patsubst %.c,%.o,$(notdir $(SOURCES))))

# Debug logging build configuration (debug + -DDEBUG for DEBUG_LOG macros)
CFLAGS_DEBUG_LOG  := $(CFLAGS_DEBUG_BASE) -DDEBUG
TARGET_DEBUG_LOG  := mini-ccstatus-debug-log
OBJ_DIR_DEBUG_LOG := $(OBJ_DIR)/debug-log
OBJECTS_DEBUG_LOG := $(addprefix $(OBJ_DIR_DEBUG_LOG)/, $(patsubst %.c,%.o,$(notdir $(SOURCES))))

# Test scripts
DEMO_QUIET_SCRIPT   := $(TST_DIR)/stdout_quiet.sh
DEMO_VERBOSE_SCRIPT := $(TST_DIR)/stdout_verbose.sh
UNIT_TEST_SCRIPT    := $(TST_DIR)/run_unit_tests.sh
TEST_SCRIPT         := $(TST_DIR)/coverage.sh
TEST_MEMORY         := $(TST_DIR)/memory.sh
TEST_VALGRIND       := $(TST_DIR)/valgrind.sh

# Test fixtures
FIXTURES := fixtures/*.json

default: $(BIN_DIR)/$(TARGET) demo-simple

.PHONY: all
all: clean build-all demo-all test valgrind

.PHONY: build-all
build-all: $(BIN_DIR)/$(TARGET) $(BIN_DIR)/$(TARGET_DEBUG) $(BIN_DIR)/$(TARGET_DEBUG_LOG)

.PHONY: demo-all
demo-all: log-dir demo-simple demo-debug

.PHONY: log-dir
log-dir:
	@mkdir -p $(LOG_DIR)

# Main target
$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Tell Make where to find source files
vpath %.c . $(SRC_DIR) $(LIB_DIR)/cjson

# Pattern rule for release build (VPATH handles source file lookup)
$(OBJ_DIR_RELEASE)/%.o: %.c $(COMMON_DEPS) | $(OBJ_DIR_RELEASE)
	$(CC) $(COMPILE_FLAGS) $(CFLAGS) $(WARNFLAGS) -c $< -o $@

$(OBJ_DIR_RELEASE) $(OBJ_DIR_DEBUG) $(OBJ_DIR_DEBUG_LOG) $(BIN_DIR):
	mkdir -p $@

# Debug build target
$(BIN_DIR)/$(TARGET_DEBUG): $(OBJECTS_DEBUG) | $(BIN_DIR)
	$(CC) $(OBJECTS_DEBUG) $(LDFLAGS) -o $@

# Pattern rule for debug build (VPATH handles source file lookup)
$(OBJ_DIR_DEBUG)/%.o: %.c $(COMMON_DEPS) | $(OBJ_DIR_DEBUG)
	$(CC) $(COMPILE_FLAGS) $(CFLAGS_DEBUG) -c $< -o $@

.PHONY: debug
debug: $(BIN_DIR)/$(TARGET_DEBUG)

# Debug logging build target (with -DDEBUG for DEBUG_LOG macros)
$(BIN_DIR)/$(TARGET_DEBUG_LOG): $(OBJECTS_DEBUG_LOG) | $(BIN_DIR)
	$(CC) $(OBJECTS_DEBUG_LOG) $(LDFLAGS) -o $@

# Pattern rule for debug-log build (VPATH handles source file lookup)
$(OBJ_DIR_DEBUG_LOG)/%.o: %.c $(COMMON_DEPS) | $(OBJ_DIR_DEBUG_LOG)
	$(CC) $(COMPILE_FLAGS) $(CFLAGS_DEBUG_LOG) -c $< -o $@

.PHONY: debug-log
debug-log: $(BIN_DIR)/$(TARGET_DEBUG_LOG)

.PHONY: test
test: $(BIN_DIR)/$(TARGET) $(UNIT_TEST_SCRIPT) $(TEST_SCRIPT) $(TEST_MEMORY) $(FIXTURES) log-dir
	@echo "Running tests (logs: $(LOG_DIR)/test-*.log)..."
	$(UNIT_TEST_SCRIPT) 2>&1 | tee $(LOG_DIR)/test-unit.log
	$(TEST_SCRIPT) 2>&1 | tee $(LOG_DIR)/test-coverage.log
	$(TEST_MEMORY) 2>&1 | tee $(LOG_DIR)/test-memory.log

.PHONY: demo-simple
demo-simple: $(BIN_DIR)/$(TARGET) $(DEMO_QUIET_SCRIPT) $(DEMO_VERBOSE_SCRIPT)
	$(DEMO_QUIET_SCRIPT)
	$(DEMO_VERBOSE_SCRIPT)

.PHONY: demo-debug
demo-debug: $(BIN_DIR)/$(TARGET_DEBUG_LOG) log-dir
	@echo "Running with debug logging enabled..."
	@cat fixtures/status.json | $(BIN_DIR)/$(TARGET_DEBUG_LOG) > $(LOG_DIR)/demo-debug-basic.log 2>&1
	@cat $(LOG_DIR)/demo-debug-basic.log
	@echo ""
	@echo "Running with --all flag and debug logging..."
	@cat fixtures/test_status_with_transcript.json | $(BIN_DIR)/$(TARGET_DEBUG_LOG) --all > $(LOG_DIR)/demo-debug-all.log 2>&1
	@cat $(LOG_DIR)/demo-debug-all.log
	@echo ""
	@echo "Debug logs saved to $(LOG_DIR)/demo-debug-*.log"

.PHONY: valgrind
valgrind: $(TEST_VALGRIND) log-dir
	@echo "Running valgrind tests (logs: $(LOG_DIR)/test-valgrind.log)..."
	$(TEST_VALGRIND) 2>&1 | tee $(LOG_DIR)/test-valgrind.log

# Static analysis targets
.PHONY: lint
lint:
	@echo "Running clang-tidy static analysis..."
	@clang-tidy $(SOURCES) -- -I. -I$(LIB_DIR) $(CPPFLAGS) $(WARNFLAGS)

.PHONY: analyze
analyze: debug
	@echo "Running GCC static analyzer..."
	@$(MAKE) clean
	@$(MAKE) debug 2>&1 | grep -E '(warning:|error:)' || echo "No analyzer warnings found"

.PHONY: clang
clang:
	@echo "Generating compile_commands.json for clangd..."
	-@bear -- $(MAKE) clean
	-@bear -- $(MAKE)
	@echo "compile_commands.json generated successfully"

.PHONY: clean
clean:
	rm -rfv $(BIN_DIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG) $(OBJ_DIR_DEBUG_LOG) $(LOG_DIR) compile_commands.json
