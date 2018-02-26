
BIN_DIR = bin
SOURCE_DIR = tests
OBJECT_DIR_DEBUG = obj
INCLUDE_DIR = include

ifeq ($(OS),Windows_NT)
	CXX = g++
	CXXFLAGS := -std=c++11 -g
	LDFLAGS := 
else 
	# OSX
	CLANG =  /Users/ben/bin/clang-5.0.1
	CXX := $(CLANG)/bin/clang++
	LLVMCONFIG := $(CLANG)/bin/llvm-config
	CXXFLAGS2 := -std=c++11 # -O3
	DEFAULTFLAGS := -I$(CLANG)/include -Wall
	CXXFLAGS := -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include $(DEFAULTFLAGS) $(CXXFLAGS2)
	LDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))
endif

OBJECTS = tests.o 		\
	array2d.o 			\
	inlined_vector.o 	\
	fixed_map.o 		\
	fixed_string.o 		\
	object_pool.o 		\
	ring_buffer.o		\
	storage_pool.o

OBJECTS_DEBUG = $(addprefix $(OBJECT_DIR_DEBUG)/, $(OBJECTS))

$(OBJECT_DIR_DEBUG)/%.o: $(SOURCE_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	@$(CXX) -o $@ -c $< -I $(INCLUDE_DIR) $(CXXFLAGS)
	
$(OBJECT_DIR_DEBUG)/tests.o: $(SOURCE_DIR)/tests.cpp
	@$(CXX) -o $@ -c $< -I $(INCLUDE_DIR) $(CXXFLAGS)

tests: $(OBJECTS_DEBUG) Makefile
	@mkdir -p $(BIN_DIR)
	@$(CXX) -o $(BIN_DIR)/$@ $(OBJECTS_DEBUG) -I $(INCLUDE_DIR) $(CXXFLAGS) $(LDFLAGS)

