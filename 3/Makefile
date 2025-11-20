# === Compiler settings ===
CXX       = g++
CXXFLAGS  = -O3 -std=c++17 -Wall -Wextra
LDFLAGS   = -lpthread

# === Project setup ===
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Source files
SOURCES = main.cpp simulation_task.cpp shock_wave_simulation.cpp task_queue.cpp physics_utils.cpp
OBJECTS = $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)
HEADERS = $(SRC_DIR)/task.h $(SRC_DIR)/task_queue.h $(SRC_DIR)/worker.h $(SRC_DIR)/simulation_task.h $(SRC_DIR)/shock_wave_simulation.h $(SRC_DIR)/physics_utils.h

# === Targets ===
TARGET = $(BUILD_DIR)/shock_simulation
TEST_WORKPOOL_TARGET = $(BUILD_DIR)/test_workpool
TEST_PHYSICS_TARGET = $(BUILD_DIR)/test_physics

# === Rules ===
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_WORKPOOL_TARGET): $(SRC_DIR)/test_workpool.cpp $(SRC_DIR)/task_queue.cpp $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/test_workpool.cpp $(SRC_DIR)/task_queue.cpp -o $@ $(LDFLAGS)

$(TEST_PHYSICS_TARGET): $(SRC_DIR)/test_physics.cpp $(SRC_DIR)/physics_utils.cpp $(SRC_DIR)/physics_utils.h
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/test_physics.cpp $(SRC_DIR)/physics_utils.cpp -o $@ $(LDFLAGS)

# === Run targets ===
run: $(TARGET)
	$(TARGET)

# === Test targets ===
test: test-workpool test-physics

test-workpool: $(TEST_WORKPOOL_TARGET)
	$(TEST_WORKPOOL_TARGET)

test-physics: $(TEST_PHYSICS_TARGET)
	$(TEST_PHYSICS_TARGET)

# === Clean ===
clean:
	rm -rf $(BUILD_DIR)

# === Help ===
help:
	@echo "Available targets:"
	@echo "  all          - Build the main simulation (default)"
	@echo "  test         - Build and run all tests"
	@echo "  test-workpool - Build and run workpool tests"
	@echo "  test-physics - Build and run physics tests"
	@echo "  run          - Build and run the simulation"
	@echo "  clean        - Remove build directory"
	@echo "  help         - Show this help message"

.PHONY: all run test test-workpool test-physics clean help
