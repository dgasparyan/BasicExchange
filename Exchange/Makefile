CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -g -O0 -Iinclude

# Boost library settings
BOOST_ROOT ?= /opt/homebrew
BOOST_INCLUDE_DIR = $(BOOST_ROOT)/include
BOOST_LIB_DIR = $(BOOST_ROOT)/lib

# Add Boost include path
CXXFLAGS += -I$(BOOST_INCLUDE_DIR)

# Boost libraries to link (add the ones you need)
BOOST_LIBS =
#-lboost_filesystem -lboost_thread

# Linker flags
LDFLAGS = -L$(BOOST_LIB_DIR) $(BOOST_LIBS)

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

TARGET = $(BIN_DIR)/program

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Default port if not specified
PORT ?= 8080

run: all
	@echo "Starting UDP server on port $(PORT)..."
	./$(TARGET) $(PORT)

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Available targets:"
	@echo "  all    - Build the program (default)"
	@echo "  run    - Build and run the program"
	@echo "  clean  - Remove build directory"
	@echo "  help   - Show this help message"
	@echo ""
	@echo "Boost configuration:"
	@echo "  BOOST_ROOT=$(BOOST_ROOT)"
	@echo "  BOOST_INCLUDE_DIR=$(BOOST_INCLUDE_DIR)"
	@echo "  BOOST_LIB_DIR=$(BOOST_LIB_DIR)"
	@echo "  BOOST_LIBS=$(BOOST_LIBS)"

.PHONY: all run clean help