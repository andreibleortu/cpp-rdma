# Directories
SRC_DIR = src
INCLUDE_DIR = include
LIB_DIR = lib
BIN_DIR = bin
OBJ_DIR = obj

# Create directories if they don't exist
$(shell mkdir -p $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR))
$(shell mkdir -p $(OBJ_DIR)/core $(OBJ_DIR)/cpp $(OBJ_DIR)/modes)
$(shell mkdir -p $(OBJ_DIR)/modes/lambda $(OBJ_DIR)/modes/read $(OBJ_DIR)/modes/write $(OBJ_DIR)/modes/send-recv $(OBJ_DIR)/examples)

# Compiler and flags
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -g -I$(INCLUDE_DIR)
CXXFLAGS = -Wall -Wextra -O2 -g -I$(INCLUDE_DIR) -std=c++11
LIBS = -libverbs -lpthread -ldl

# C core sources
C_CORE_SRC = $(SRC_DIR)/core/common.c \
             $(SRC_DIR)/core/rdma.c

# RDMA mode-specific sources
MODE_SEND_RECV_SRC = $(SRC_DIR)/modes/send-recv/send_receive.c
MODE_WRITE_SRC = $(SRC_DIR)/modes/write/rdma_write.c
MODE_READ_SRC = $(SRC_DIR)/modes/read/rdma_read.c
MODE_LAMBDA_SRC = $(SRC_DIR)/modes/lambda/lambda_client.c \
                 $(SRC_DIR)/modes/lambda/lambda_server.c

# C++ wrapper sources
CXX_SRC = $(SRC_DIR)/cpp/rdma_connection.cpp \
          $(SRC_DIR)/cpp/rdma-vector.cpp

# Example sources
C_EXAMPLES_SRC = $(SRC_DIR)/examples/lambda-run.c
CXX_EXAMPLES_SRC = $(SRC_DIR)/examples/rdma-example.cpp

# Object files
C_CORE_OBJ = $(patsubst $(SRC_DIR)/core/%.c,$(OBJ_DIR)/core/%.o,$(C_CORE_SRC))
MODE_SEND_RECV_OBJ = $(patsubst $(SRC_DIR)/modes/send-recv/%.c,$(OBJ_DIR)/modes/send-recv/%.o,$(MODE_SEND_RECV_SRC))
MODE_WRITE_OBJ = $(patsubst $(SRC_DIR)/modes/write/%.c,$(OBJ_DIR)/modes/write/%.o,$(MODE_WRITE_SRC))
MODE_READ_OBJ = $(patsubst $(SRC_DIR)/modes/read/%.c,$(OBJ_DIR)/modes/read/%.o,$(MODE_READ_SRC))
MODE_LAMBDA_OBJ = $(patsubst $(SRC_DIR)/modes/lambda/%.c,$(OBJ_DIR)/modes/lambda/%.o,$(MODE_LAMBDA_SRC))
CXX_OBJ = $(patsubst $(SRC_DIR)/cpp/%.cpp,$(OBJ_DIR)/cpp/%.o,$(CXX_SRC))
C_EXAMPLES_OBJ = $(patsubst $(SRC_DIR)/examples/%.c,$(OBJ_DIR)/examples/%.o,$(C_EXAMPLES_SRC))
CXX_EXAMPLES_OBJ = $(patsubst $(SRC_DIR)/examples/%.cpp,$(OBJ_DIR)/examples/%.o,$(CXX_EXAMPLES_SRC))

# All C objects
C_OBJS = $(C_CORE_OBJ) $(MODE_SEND_RECV_OBJ) $(MODE_WRITE_OBJ) $(MODE_READ_OBJ) $(MODE_LAMBDA_OBJ)

# Main targets
.PHONY: all rdma rdmacpp clean

all: rdma rdmacpp $(BIN_DIR)/lambda-run.so

rdma: $(BIN_DIR)/rdma

rdmacpp: $(BIN_DIR)/rdma-example

# Main RDMA executable
$(BIN_DIR)/rdma: $(C_OBJS)
	$(CC) $^ -o $@ $(LIBS)

# C++ example
$(BIN_DIR)/rdma-example: $(CXX_EXAMPLES_OBJ) $(CXX_OBJ) $(OBJ_DIR)/core/common.o
	$(CXX) $^ -o $@ $(LIBS)

# Lambda function shared library
$(BIN_DIR)/lambda-run.so: $(OBJ_DIR)/examples/lambda-run.o
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $<

# Object file compilation rules
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/modes/send-recv/%.o: $(SRC_DIR)/modes/send-recv/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/modes/write/%.o: $(SRC_DIR)/modes/write/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/modes/read/%.o: $(SRC_DIR)/modes/read/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/modes/lambda/%.o: $(SRC_DIR)/modes/lambda/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/cpp/%.o: $(SRC_DIR)/cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/examples/%.o: $(SRC_DIR)/examples/%.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(OBJ_DIR)/examples/%.o: $(SRC_DIR)/examples/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*