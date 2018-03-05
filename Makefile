
SOURCE_DIR = tests
INCLUDE_DIR = include
BIN_DIR_ROOT = bin
OBJ_DIR_ROOT = obj
CXXFLAGS = -std=c++11 -Wall

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g3 -DDEBUG
	CONFIG = debug
else
    CXXFLAGS += -DNDEBUG
	CONFIG = release
endif

OBJ_DIR = $(OBJ_DIR_ROOT)/$(CONFIG)
BIN_DIR = $(BIN_DIR_ROOT)/$(CONFIG)

OBJS = tests.o \
	array2d.o \
	inlined_vector.o \
	fixed_map.o \
	fixed_string.o \
	object_pool.o \
	ring_buffer.o

ALL_OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS))

.PHONY: all clean test build-test run-test
all: build-test

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	@mkdir -p $(OBJ_DIR)
	$(CXX) -o $@ -c $< -I $(INCLUDE_DIR) $(CXXFLAGS)
	
$(OBJ_DIR)/tests.o: $(SOURCE_DIR)/tests.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -o $@ -c $< -I $(INCLUDE_DIR) $(CXXFLAGS)

clean:
	rm -rf $(OBJ_DIR_ROOT)
	rm -rf $(BIN_DIR_ROOT)

test: $(BIN_DIR)/test run-test

build-test: $(BIN_DIR)/test

$(BIN_DIR)/test: $(ALL_OBJS) Makefile
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $(BIN_DIR)/test $(ALL_OBJS) -I $(INCLUDE_DIR) $(CXXFLAGS) $(LDFLAGS)

run-test:
	$(BIN_DIR)/test


