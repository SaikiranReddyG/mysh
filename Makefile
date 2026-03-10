# mysh Makefile
# Compiles all source files and links into mysh binary

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
DEBUG_FLAGS = -g -fsanitize=address

SRC_DIR = src
OBJS = $(SRC_DIR)/main.o $(SRC_DIR)/parser.o $(SRC_DIR)/executor.o \
       $(SRC_DIR)/builtins.o $(SRC_DIR)/redirect.o $(SRC_DIR)/pipes.o \
       $(SRC_DIR)/signals.o $(SRC_DIR)/jobs.o

TARGET = mysh

# Default target
.PHONY: all
all: $(TARGET)

# Debug build with AddressSanitizer
.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Link binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/shell.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean compiled artifacts
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f core vgcore.*

# Phony targets
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make          - Build normally"
	@echo "  make debug    - Build with AddressSanitizer (memory debugging)"
	@echo "  make clean    - Remove compiled files"
	@echo "  make help     - Show this message"
