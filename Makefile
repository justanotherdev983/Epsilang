# Compiler
CC = gcc

# Directories
SRC_DIR = src
PARSE_DIR = $(SRC_DIR)/parse
TOKENIZE_DIR = $(SRC_DIR)/tokenize
OUTPUT_DIR = output

# Source files
SRCS = $(SRC_DIR)/main.c $(PARSE_DIR)/parse.c $(TOKENIZE_DIR)/tokenize.c

# Object files
OBJS = $(OUTPUT_DIR)/main.o $(OUTPUT_DIR)/parse.o $(OUTPUT_DIR)/tokenize.o

# Output files
OUTPUT = $(OUTPUT_DIR)/z
ASM_OUTPUT = $(OUTPUT_DIR)/output.asm
OBJ_OUTPUT = $(OUTPUT_DIR)/output.o

# Compiler flags
CFLAGS = -I$(PARSE_DIR) -I$(TOKENIZE_DIR) -Wall

# Default target
all: $(OUTPUT)

# Link the object files to create the final executable
$(OUTPUT): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compile main.c to main.o
$(OUTPUT_DIR)/main.o: $(SRC_DIR)/main.c $(PARSE_DIR)/parse.h $(TOKENIZE_DIR)/tokenize.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compile parse.c to parse.o
$(OUTPUT_DIR)/parse.o: $(PARSE_DIR)/parse.c $(PARSE_DIR)/parse.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compile tokenize.c to tokenize.o
$(OUTPUT_DIR)/tokenize.o: $(TOKENIZE_DIR)/tokenize.c $(TOKENIZE_DIR)/tokenize.h
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble output.asm to output.o (assuming you have an assembler step)
$(OBJ_OUTPUT): $(ASM_OUTPUT)
	$(AS) $(ASM_OUTPUT) -o $@

# Clean up build files
clean:
	rm -f $(OUTPUT) $(OBJS) $(ASM_OUTPUT) $(OBJ_OUTPUT)

.PHONY: all clean
