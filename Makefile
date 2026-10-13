CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wno-unused-parameter
CPPFLAGS += -Isrc -Iexternal/csv2/include
HEADERS := $(shell find src external/csv2/include -name '*.h' -o -name '*.hpp')
.PHONY: all test smoke demo demo-verify clean
all: build/trading_system
build:
	mkdir -p build
build/trading_system: src/main.cpp src/data/DataManager.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp src/data/DataManager.cpp -o $@
build/test_circular_buffer: test_circular_buffer.cpp src/core/Utils.h | build
	$(CXX) $(CXXFLAGS) $< -o $@
test: build/test_circular_buffer
	./build/test_circular_buffer
smoke: build/trading_system
	./build/trading_system --max-rows=100
demo: build/trading_system
	python3 demo/generate_demo.py
demo-verify:
	python3 demo/verify_demo.py
clean:
	$(RM) build/trading_system build/test_circular_buffer
