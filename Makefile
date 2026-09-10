CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
OBJ_DIR = bin/obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp) \
       $(wildcard $(SRC_DIR)/*/*.cpp)

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/aethelgard.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build successful: $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Cleaned build artifacts."

.PHONY: all clean
