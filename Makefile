# Compiler settings
CC = gcc
CFLAGS = -Wall -fPIC
INCLUDES = -I include -I include/common
LDFLAGS = -shared

# Directories
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib

# Common library name
COMMON_LIB = $(LIB_DIR)/libcommon.so

# Common source files
COMMON_SRCS = $(SRC_DIR)/common/document.c $(SRC_DIR)/common/indexer.c
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Create shared library
common: $(COMMON_LIB)

$(COMMON_LIB): $(COMMON_OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) $(LDFLAGS) $(INCLUDES) -o $@ $^

# Pattern rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR)

.PHONY: common clean