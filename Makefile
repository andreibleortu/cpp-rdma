# Directories
SRC_DIR = src
INCLUDE_DIR = include
LIB_DIR = lib
BIN_DIR = bin
OBJ_DIR = obj

# Create directories if they don't exist
$(shell mkdir -p $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR))
$(shell mkdir -p $(OBJ_DIR)/core $(OBJ_DIR)/cpp $(OBJ_DIR)/examples)

# Compiler and flags
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -g -I$(INCLUDE_DIR)
CXXFLAGS = -Wall -Wextra -O2 -g -I$(INCLUDE_DIR) -std=c++11
LIBS = -libverbs -lpthread

# C core sources (support for C++ layer)
C_CORE_SRC = $(SRC_DIR)/core/common.c

# C++ wrapper sources
CXX_SRC = $(SRC_DIR)/cpp/rdma_connection.cpp \
          $(SRC_DIR)/cpp/rdma-vector.cpp

# Example sources
CXX_EXAMPLES_SRC = $(SRC_DIR)/examples/rdma-example.cpp

# Object files
C_CORE_OBJ = $(patsubst $(SRC_DIR)/core/%.c,$(OBJ_DIR)/core/%.o,$(C_CORE_SRC))
CXX_OBJ = $(patsubst $(SRC_DIR)/cpp/%.cpp,$(OBJ_DIR)/cpp/%.o,$(CXX_SRC))
CXX_EXAMPLES_OBJ = $(patsubst $(SRC_DIR)/examples/%.cpp,$(OBJ_DIR)/examples/%.o,$(CXX_EXAMPLES_SRC))

# Main targets
.PHONY: all clean

all: $(BIN_DIR)/rdma-example $(BIN_DIR)/rdma

# C++ RDMA example (main binary)
$(BIN_DIR)/rdma-example: $(CXX_EXAMPLES_OBJ) $(CXX_OBJ) $(C_CORE_OBJ)
	$(CXX) $^ -o $@ $(LIBS)

# Create rdma as a symlink to rdma-example
$(BIN_DIR)/rdma: $(BIN_DIR)/rdma-example
	ln -sf rdma-example $(BIN_DIR)/rdma

# Object file compilation rules
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/cpp/%.o: $(SRC_DIR)/cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/examples/%.o: $(SRC_DIR)/examples/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*
