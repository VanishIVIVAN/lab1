CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude
SRC      := src/elements.cpp src/document.cpp src/rules.cpp src/session.cpp src/shell.cpp src/demo.cpp
OBJ      := $(SRC:src/%.cpp=build/%.o)

all: build/lab1 build/lab1_tests

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/lab1: $(OBJ) src/main.cpp
	$(CXX) $(CXXFLAGS) $(OBJ) src/main.cpp -o $@

build/lab1_tests: $(OBJ) tests/tests.cpp
	$(CXX) $(CXXFLAGS) $(OBJ) tests/tests.cpp -o $@

test: build/lab1_tests
	./build/lab1_tests

clean:
	rm -rf build

.PHONY: all test clean
