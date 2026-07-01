# Choose compiler and set compiler flags
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Set build, and source directories
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src

# Set targets and objects
TARGET = $(BIN_DIR)/project
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run project entrypoint
run:
	./$(TARGET)

# Clean build files
clean:
	@for dir in $(BUILD_DIR) $(BIN_DIR); do \
		rm -rf "$$dir"/*; \
	done

# Clang style formatting
format:
	clang-format -i $(SRCS)

.PHONY: all clean
