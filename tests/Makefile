CLANG =  /Users/ben/bin/clang-5.0.1

CXX := $(CLANG)/bin/clang++
LLVMCONFIG := $(CLANG)/bin/llvm-config
CXXFLAGS2 := -std=c++11 # -O3
DEFAULTFLAGS := -I$(CLANG)/include -Wall
CXXFLAGS := -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include $(DEFAULTFLAGS) $(CXXFLAGS2)
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))
INC = -I ../include
TEST?=

SRC = tests.cpp fixed_map.cpp fixed_string.cpp inlined_vector.cpp ring_buffer.cpp array2d.cpp

tests: $(SRC) Makefile ../include/*
	@$(CXX) -o ../bin/$@ $(SRC) $(INC) $(CXXFLAGS) $(LLVMLDFLAGS)

run: tests
	../bin/tests $(TEST)
