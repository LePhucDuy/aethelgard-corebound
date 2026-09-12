CXX ?= g++
SRC_DIR = src
OBJ_DIR = bin/obj
BIN_DIR = bin

# Phat he nhe dieu hanh de dung flag link tuong ung
ifeq ($(OS),Windows_NT)
    # Windows (MSYS2/MinGW): dung raylib he thong, xuat file .exe
    CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
    LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET = $(BIN_DIR)/aethelgard.exe
else
    # Linux: raylib/GLFW static duoc build san trong lib/
    CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -Ilib/include
    LDFLAGS = lib/libraylib.a lib/libglfw3.a \
              -lX11 -lGL -lm -lpthread -ldl \
              -lXrandr -lXi -lXcursor -lXinerama -lXfixes -lXext
    TARGET = $(BIN_DIR)/aethelgard
endif

SRCS = $(wildcard $(SRC_DIR)/*.cpp) \
       $(wildcard $(SRC_DIR)/*/*.cpp)

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@if [ -z "$$OS" ] && [ ! -f lib/libraylib.a ]; then \
		echo "[ERROR] Khong tim thay lib/libraylib.a (thu vien raylib cho Linux)."; \
		echo "        Hay chay:  ./setup-linux.sh   (chi tiet trong README.md)"; \
		exit 1; \
	fi
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build successful: $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

setup:
	@bash setup-linux.sh $(SETUP_ARGS)

run: $(TARGET)
	./$(TARGET)

clean:
	@rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Cleaned build artifacts."

.PHONY: all setup run clean

