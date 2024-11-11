# Compiler settings
CC = gcc
CFLAGS = -Wall -fPIC
INCLUDES = -I include -I include/common -I include/indexer -I include/server
LDFLAGS = -shared
BINFLAGS = -L$(LIB_DIR) -lcommon

# Directories
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
BIN_DIR = bin

# Common library
COMMON_LIB = $(LIB_DIR)/libcommon.so
COMMON_SRCS = $(SRC_DIR)/common/document.c $(SRC_DIR)/common/indexer.c
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Indexer executable
INDEXER_SRCS = $(SRC_DIR)/indexer/indexer_main.c
INDEXER_OBJS = $(INDEXER_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
INDEXER_BIN = $(BIN_DIR)/indexer

# Server executable
SERVER_SRCS = $(SRC_DIR)/server/server.c
SERVER_OBJS = $(SERVER_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SERVER_BIN = $(BIN_DIR)/server

# Targets
all: common indexer server

common: $(COMMON_LIB)

indexer: common $(INDEXER_BIN)

server: common $(SERVER_BIN)

# Build rules
$(COMMON_LIB): $(COMMON_OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) $(LDFLAGS) $(INCLUDES) -o $@ $^

$(INDEXER_BIN): $(INDEXER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(INCLUDES) -o $@ $^ $(BINFLAGS)

$(SERVER_BIN): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(INCLUDES) -o $@ $^ $(BINFLAGS)

# Pattern rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

.PHONY: all common indexer server clean