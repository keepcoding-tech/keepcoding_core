# This file is part of keepcoding_core
# ==================================
#
# makefile
#
# Copyright (c) 2024 Daniel Tanase
# SPDX-License-Identifier: MIT License
#
#                  _         __ _ _
#                 | |       / _(_) |
#  _ __ ___   __ _| | _____| |_ _| | ___
# | '_ ` _ \ / _` | |/ / _ \  _| | |/ _ \
# | | | | | | (_| |   <  __/ | | | |  __/
# |_| |_| |_|\__,_|_|\_\___|_| |_|_|\___|
#

# Specify the compiler and compiler flags
CC     := gcc
STD    := -std=c99 -std=gnu99
CFLAGS := -Wall -Werror -Wpedantic -g -pthread

# Specify the sources and the headers directory
BLD_DIR  := build
HDR_DIR  := hdrs
SRC_DIR  := srcs
TST_DIR  := test
LOG_DIR  := $(BLD_DIR)/log
OBJ_DIR  := $(BLD_DIR)/obj
LIB_DIR  := $(BLD_DIR)/lib
BIN_DIR  := $(BLD_DIR)/bin
BIN_TST_DIR  := $(BIN_DIR)/test

# Specify the sources and headers files
HEADERS := $(wildcard $(HDR_DIR)/*.h) $(wildcard $(HDR_DIR)/**/*.h)
SOURCES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)
TESTS   := $(wildcard $(TST_DIR)/*.c)

.PHONY: all build test run_tests clean help

##################################### ALL ######################################

all: clean build test run_tests

#################################### BUILD #####################################

# Create object file names by replacing .c with .o in SOURCES
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

build: $(LIB_DIR)/libkeepcoding.a

$(LIB_DIR)/libkeepcoding.a: $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

# Rule for generating object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(STD) $(CFLAGS) -c $< -o $@

##################################### TEST #####################################

# Generate corresponding executable names
TEST_EXECUTABLES := $(patsubst $(TST_DIR)/%.c,$(BIN_TST_DIR)/%,$(TESTS))

test: $(TEST_EXECUTABLES)

$(BIN_TST_DIR)/%: $(TST_DIR)/%.c $(LIB_DIR)/libkeepcoding.a
	@mkdir -p $(dir $@)
	$(CC) $(STD) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lkeepcoding

run_tests: $(TEST_EXECUTABLES)
	@mkdir -p $(LOG_DIR)
	@$(foreach test_executable, $(TEST_EXECUTABLES), $(test_executable);)

#################################### CLEAN #####################################

clean:
	rm -fr $(BLD_DIR)

##################################### HELP #####################################

help:
	@echo "Available targets:"
	@echo "  all         : Compile the static libraries and all test executables"
	@echo "  build       : Compile the static libraries"
	@echo "  test        : Compile and run all test executables consecutively"
	@echo "  clean       : Clean up the object files and build directory"
	@echo "  help        : Display this help message"

