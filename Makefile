.PHONY: all clean

CC		:= /opt/gcc-9.2/bin/g++
OLD_CC  := g++
C_FLAGS := -std=c++17 -O3 -g -pthread -Wall -Wextra
C_FLAGS_DEBUG := -std=c++17 -O0 -g -pthread -Wall -Wextra

BIN			    := bin
SRC			    := src
INCLUDE		    := include
THIRD_PARTY	    := third_party/matplotlib-cpp
HT_TESTS		:= $(SRC)/tests/hash_table/main.cpp
SET_TESTS		:= $(SRC)/tests/set/main.cpp
HT_BENCHMARK    := $(SRC)/benchmark/hash_table/main.cpp
SET_BENCHMARK   := $(SRC)/benchmark/set/main.cpp
PYTHON 		    := python3.6m
PYTHON_INCLUDE  := -I/usr/include/$(PYTHON)/ -I/usr/lib64/python3.6/site-packages/numpy/core/include
PYTHON_LDFLAGS  := -l$(PYTHON)

HT_TESTS_EXECUTABLE := $(BIN)/ht_tests.out
SET_TESTS_EXECUTABLE := $(BIN)/set_tests.out
HT_BENCHMARK_EXECUTABLE := $(BIN)/ht_benchmark.out
SET_BENCHMARK_EXECUTABLE := $(BIN)/set_benchmark.out

all: $(HT_TESTS_EXECUTABLE) $(HT_BENCHMARK_EXECUTABLE) $(SET_TESTS_EXECUTABLE) $(SET_BENCHMARK_EXECUTABLE)


clean:
	$(RM) $(BIN)/*

run: all

$(HT_BENCHMARK_EXECUTABLE): $(HT_BENCHMARK)
	$(CC) $(C_FLAGS) -I$(INCLUDE) $(PYTHON_INCLUDE) -I$(THIRD_PARTY) $^ -o $@ $(PYTHON_LDFLAGS) 

$(HT_TESTS_EXECUTABLE): $(HT_TESTS)
#	$(CC) $(C_FLAGS) -I$(INCLUDE) $^ -o $@ 

$(SET_BENCHMARK_EXECUTABLE): $(SET_BENCHMARK)
	$(CC) $(C_FLAGS) -I$(INCLUDE) $(PYTHON_INCLUDE) -I$(THIRD_PARTY) $^ -o $@ $(PYTHON_LDFLAGS) 

$(SET_TESTS_EXECUTABLE): $(SET_TESTS)
	$(CC) $(C_FLAGS_DEBUG) -I$(INCLUDE) $^ -o $@ 
#$(HT_EXECUTABLE): $(TESTS)/hash_table/*
#	$(CC) $(C_FLAGS_DEBUG) -I$(INCLUDE) $(PYTHON) -I$(THIRD_PARTY) -L$(LIBRARIES) $^ -o $@ 
