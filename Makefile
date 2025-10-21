# Copyright (c) 2025 Michele Tavella <meeghele@proton.me>
# Licensed under the MIT License. See LICENSE file for details.

# Colors are enabled by default; set NO_COLOR in the environment to disable.
CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -O3 -pipe -march=native -flto -DNDEBUG
WARNFLAGS ?= -Wall -Wextra -Wpedantic -Wno-sign-conversion
LDFLAGS ?=

TARGET := mini-ccstatus
OBJ_DIR := obj
BIN_DIR := bin
OBJECTS := $(OBJ_DIR)/mini-ccstatus.o $(OBJ_DIR)/cJSON.o

# Debug build configuration (for valgrind and debugging)
CFLAGS_DEBUG := -g -O0 $(WARNFLAGS)
TARGET_DEBUG := mini-ccstatus-debug
OBJ_DIR_DEBUG := obj-debug
OBJECTS_DEBUG := $(OBJ_DIR_DEBUG)/mini-ccstatus.o $(OBJ_DIR_DEBUG)/cJSON.o

# Test scripts
DEMO_SCRIPT := tests/stdout.sh
TEST_SCRIPT := tests/coverage.sh
TEST_MEMORY := tests/memory.sh
TEST_VALGRIND := tests/valgrind.sh
FIXTURES := fixtures/*.json

default: $(BIN_DIR)/$(TARGET) demo

.PHONY: all
all: clean default test valgrind

$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJ_DIR)/mini-ccstatus.o: mini-ccstatus.c lib/cjson/cJSON.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNFLAGS) -I. -Ilib -c $< -o $@

$(OBJ_DIR)/cJSON.o: lib/cjson/cJSON.c lib/cjson/cJSON.h | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNFLAGS) -I. -Ilib -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Debug build targets
$(BIN_DIR)/$(TARGET_DEBUG): $(OBJECTS_DEBUG) | $(BIN_DIR)
	$(CC) $(OBJECTS_DEBUG) $(LDFLAGS) -o $@

$(OBJ_DIR_DEBUG)/mini-ccstatus.o: mini-ccstatus.c lib/cjson/cJSON.h | $(OBJ_DIR_DEBUG)
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -I. -Ilib -c $< -o $@

$(OBJ_DIR_DEBUG)/cJSON.o: lib/cjson/cJSON.c lib/cjson/cJSON.h | $(OBJ_DIR_DEBUG)
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -I. -Ilib -c $< -o $@

$(OBJ_DIR_DEBUG):
	mkdir -p $@

.PHONY: debug
debug: $(BIN_DIR)/$(TARGET_DEBUG)

.PHONY: test
test: $(BIN_DIR)/$(TARGET) $(TEST_SCRIPT) $(TEST_MEMORY) $(FIXTURES)
	$(TEST_SCRIPT)
	$(TEST_MEMORY)

.PHONY: demo
demo: $(BIN_DIR)/$(TARGET) $(DEMO_SCRIPT)
	$(DEMO_SCRIPT)
	VERBOSE=true $(DEMO_SCRIPT)

.PHONY: valgrind
valgrind: $(TEST_VALGRIND)
	$(TEST_VALGRIND)

.PHONY: clean
clean:
	rm -rfv $(BIN_DIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG)
