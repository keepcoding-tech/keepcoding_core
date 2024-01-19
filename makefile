# Specify the compiler and compiler flags
CC     := gcc
STD    := -std=c99
CFLAGS := -Wall -Werror -Wpedantic -g

# Specify the sources and the headers directory
HDR_DIR  := hdrs
SRC_DIR  := srcs
BIN_DIR  := build
OBJ_DIR  := $(BIN_DIR)/obj
LIB_DIR  := $(BIN_DIR)/lib

# Specify the sources and headers files
HEADERS := $(wildcard $(HDR_DIR)/**/*.h)
SOURCES := $(wildcard $(SRC_DIR)/**/*.c)
TESTS   := $(wildcard test/**/*.c)

.PHONY: all build test clean help

##################################### ALL ######################################

all: clean build test

#################################### BUILD #####################################

# Create object file names by replacing .c with .o in SOURCES
OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SOURCES)))

# Build command
build: $(LIB_DIR)/libkeepcoding.a

$(LIB_DIR)/libkeepcoding.a: $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/**/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(STD) -c $< -o $@

##################################### TEST #####################################

test: $(LIB_DIR)/libkeepcoding.a $(TESTS)
	@for test_file in $(TESTS); do \
		test_name=$$(basename $$test_file .c); \
		$(CC) $(STD) $(CFLAGS) $$test_file -o $(BIN_DIR)/$$test_name -L$(LIB_DIR) -lkeepcoding; \
		$(BIN_DIR)/$$test_name; \
	done

#################################### CLEAN #####################################

clean:
	rm -fr $(BIN_DIR)

##################################### HELP #####################################

help:
	@echo "Available targets:"
	@echo "  all         : Compile the static libraries and run tests"
	@echo "  build       : Compile the static libraries"
	@echo "  test        : Compile and run all test files"
	@echo "  clean       : Clean up the object files and build directory"
	@echo "  help        : Display this help message"
