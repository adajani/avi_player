# Makefile for AVI Player
# Simple AVI Player for Uncompressed Video Files
# Author: Your Name
# Date: 2025

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG
INCLUDES = 
LIBS = -lSDL2

# Directories
SRC_DIR = .
BUILD_DIR = build
DOC_DIR = docs

# Source files
SOURCES = main.cpp avi_player.cpp
HEADERS = avi_player.h
OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Target executable
TARGET = avi_player

# Default target
all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the main executable
$(TARGET): $(BUILD_DIR) $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	@echo "Clean complete"

# Clean everything including documentation
distclean: clean
	rm -rf $(DOC_DIR)
	@echo "Full clean complete"

# Generate documentation with Doxygen
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in $(DOC_DIR)/"; \
	else \
		echo "Error: Doxygen not found. Please install doxygen to generate documentation."; \
		echo "Ubuntu/Debian: sudo apt-get install doxygen"; \
		echo "macOS: brew install doxygen"; \
		exit 1; \
	fi

# Install target (optional)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin"
	@if [ "$(shell id -u)" -eq 0 ]; then \
		cp $(TARGET) /usr/local/bin/; \
		chmod 755 /usr/local/bin/$(TARGET); \
		echo "Installation complete"; \
	else \
		echo "Run 'sudo make install' to install to system directory"; \
		exit 1; \
	fi

# Uninstall target
uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin"
	@if [ "$(shell id -u)" -eq 0 ]; then \
		rm -f /usr/local/bin/$(TARGET); \
		echo "Uninstall complete"; \
	else \
		echo "Run 'sudo make uninstall' to remove from system directory"; \
		exit 1; \
	fi

# Run the program with a test file (if available)
test: $(TARGET)
	@if [ -f test.avi ]; then \
		./$(TARGET) test.avi; \
	else \
		echo "No test.avi file found. Usage: ./$(TARGET) <your_video.avi>"; \
	fi

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@echo -n "SDL2 development libraries: "
	@if pkg-config --exists sdl2; then \
		echo "✓ Found (version: $$(pkg-config --modversion sdl2))"; \
	else \
		echo "✗ Not found"; \
		echo "Please install SDL2 development libraries:"; \
		echo "  Ubuntu/Debian: sudo apt-get install libsdl2-dev"; \
		echo "  CentOS/RHEL: sudo yum install SDL2-devel"; \
		echo "  macOS: brew install sdl2"; \
		exit 1; \
	fi
	@echo -n "C++ compiler: "
	@if command -v $(CXX) >/dev/null 2>&1; then \
		echo "✓ Found ($$($(CXX) --version | head -n1))"; \
	else \
		echo "✗ Not found"; \
		exit 1; \
	fi

# Show help
help:
	@echo "AVI Player Makefile"
	@echo "Available targets:"
	@echo "  all        - Build the program (default)"
	@echo "  debug      - Build with debug symbols"
	@echo "  clean      - Remove build artifacts"
	@echo "  distclean  - Remove build artifacts and documentation"
	@echo "  docs       - Generate Doxygen documentation"
	@echo "  install    - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall  - Remove from /usr/local/bin (requires sudo)"
	@echo "  test       - Run with test.avi if available"
	@echo "  check-deps - Check if required dependencies are installed"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage example:"
	@echo "  make"
	@echo "  ./$(TARGET) video.avi"

# Print build information
info:
	@echo "Build Configuration:"
	@echo "  Compiler: $(CXX)"
	@echo "  Flags: $(CXXFLAGS)"
	@echo "  Libraries: $(LIBS)"
	@echo "  Sources: $(SOURCES)"
	@echo "  Target: $(TARGET)"

# Phony targets
.PHONY: all debug clean distclean docs install uninstall test check-deps help info

# Dependencies
$(BUILD_DIR)/main.o: main.cpp avi_player.h
$(BUILD_DIR)/avi_player.o: avi_player.cpp avi_player.h