# === Compiler settings ===
CXX       = /opt/homebrew/bin/g++-15
CXXFLAGS  = -O3 -fopenmp -march=native -Wall -std=c++17
LDFLAGS   = 

# === Project setup ===
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/heat_diffusion.cpp $(SRC_DIR)/utils.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TEST_SOURCES = $(SRC_DIR)/test_validation.cpp $(SRC_DIR)/heat_diffusion.cpp $(SRC_DIR)/utils.cpp
TEST_OBJECTS = $(TEST_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# === Targets ===
TARGET = $(BUILD_DIR)/heat_diffusion
TEST_TARGET = $(BUILD_DIR)/test_validation

# === Rules ===
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(TEST_OBJECTS) -o $(TEST_TARGET) $(CXXFLAGS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# === Run targets ===
run: $(TARGET)
	cd $(BUILD_DIR) && ./heat_diffusion

# === Test targets ===
test: validate

validate: $(TEST_TARGET)
	$(TEST_TARGET)

# === Debug build ===
debug: CXXFLAGS = -g -fopenmp -Wall -std=c++17
debug: clean $(TARGET)

# === Clean ===
clean:
	rm -rf $(BUILD_DIR)

# === Help ===
help:
	@echo "Available targets:"
	@echo "  all      - Build the executable (default)"
	@echo "  test     - Run validation test"
	@echo "  validate - Run validation test"
	@echo "  run      - Build and run the program"
	@echo "  debug    - Build with debug symbols"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help message"

.PHONY: all run test validate debug clean help
