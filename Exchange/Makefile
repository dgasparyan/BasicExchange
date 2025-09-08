CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic -g -O0 -Iinclude -fexperimental-library
# -fexperimental-library is needed for stop_source and stop_token on Apple Clange 


# Boost library settings
BOOST_ROOT ?= /opt/homebrew
BOOST_INCLUDE_DIR = $(BOOST_ROOT)/include
BOOST_LIB_DIR = $(BOOST_ROOT)/lib

# Add Boost include path
CXXFLAGS += -I$(BOOST_INCLUDE_DIR)

# Test-specific flags (includes UNIT_TESTS macro for friend class access)
TEST_CXXFLAGS = $(CXXFLAGS) -I$(GTEST_INCLUDE_DIR) -DUNIT_TESTS

# Boost libraries to link (add the ones you need)
BOOST_LIBS =
#-lboost_filesystem -lboost_thread

# Google Test settings
GTEST_DIR ?= /opt/homebrew
GTEST_INCLUDE_DIR = $(GTEST_DIR)/include
GTEST_LIB_DIR = $(GTEST_DIR)/lib
GTEST_LIBS = -lgtest -lgtest_main -lgmock -lpthread

# Linker flags
LDFLAGS = -L$(BOOST_LIB_DIR) $(BOOST_LIBS)
TEST_LDFLAGS = -L$(BOOST_LIB_DIR) -L$(GTEST_LIB_DIR) $(BOOST_LIBS) $(GTEST_LIBS)

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

TARGET = $(BIN_DIR)/program
TEST_TARGET = $(BIN_DIR)/run_tests

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# Test sources exclude main.cpp
TEST_SOURCES := $(wildcard test/*.cpp)
TEST_OBJECTS := $(patsubst test/%.cpp,$(OBJ_DIR)/test_%.o,$(TEST_SOURCES))

# Test library objects (exclude main.cpp)
TEST_LIB_SOURCES := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
TEST_LIB_OBJECTS := $(patsubst src/%.cpp,$(OBJ_DIR)/test_lib_%.o,$(TEST_LIB_SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJECTS) $(TEST_LIB_OBJECTS) | $(BIN_DIR)
	$(CXX) $(TEST_OBJECTS) $(TEST_LIB_OBJECTS) -o $(TEST_TARGET) $(TEST_LDFLAGS)

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Test library objects (compiled with UNIT_TESTS macro)
$(OBJ_DIR)/test_lib_%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -DUNIT_TESTS -MMD -MP -c $< -o $@

$(OBJ_DIR)/test_%.o: test/%.cpp | $(OBJ_DIR)
	$(CXX) $(TEST_CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(OBJECTS:.o=.d)
-include $(TEST_OBJECTS:.o=.d)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Default port if not specified
PORT ?= 8080

run: all
	@echo "Starting UDP server on port $(PORT)..."
	./$(TARGET) $(PORT)

test: $(TEST_TARGET)
	@echo "Running tests..."
	./$(TEST_TARGET)

test-verbose: $(TEST_TARGET)
	@echo "Running tests with verbose output..."
	./$(TEST_TARGET) --gtest_verbose

test-filter: $(TEST_TARGET)
	@echo "Running tests with filter..."
	./$(TEST_TARGET) --gtest_filter=$(FILTER)

clean:
	rm -rf $(BUILD_DIR)

force-test: clean test

help:
	@echo "Available targets:"
	@echo "  all         - Build the program (default)"
	@echo "  run         - Build and run the program"
	@echo "  test        - Build and run all tests"
	@echo "  test-verbose- Run tests with verbose output"
	@echo "  test-filter - Run tests with filter (set FILTER=pattern)"
	@echo "  force-test  - Clean build and run all tests"
	@echo "  clean       - Remove build directory"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Boost configuration:"
	@echo "  BOOST_ROOT=$(BOOST_ROOT)"
	@echo "  BOOST_INCLUDE_DIR=$(BOOST_INCLUDE_DIR)"
	@echo "  BOOST_LIB_DIR=$(BOOST_LIB_DIR)"
	@echo "  BOOST_LIBS=$(BOOST_LIBS)"
	@echo ""
	@echo "Google Test configuration:"
	@echo "  GTEST_DIR=$(GTEST_DIR)"
	@echo "  GTEST_INCLUDE_DIR=$(GTEST_INCLUDE_DIR)"
	@echo "  GTEST_LIB_DIR=$(GTEST_LIB_DIR)"

.PHONY: all run test test-verbose test-filter force-test clean help