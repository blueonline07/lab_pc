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

# Output executables
TARGET = $(BUILD_DIR)/shock_simulation
TEST_TARGET = $(BUILD_DIR)/test_workpool
PHYSICS_TEST_TARGET = $(BUILD_DIR)/test_physics

# === Rules ===
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET) $(PHYSICS_TEST_TARGET)

$(TEST_TARGET): $(SRC_DIR)/test_workpool.cpp $(SRC_DIR)/task_queue.cpp $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/test_workpool.cpp $(SRC_DIR)/task_queue.cpp -o $@ $(LDFLAGS)

$(PHYSICS_TEST_TARGET): $(SRC_DIR)/test_physics.cpp $(SRC_DIR)/physics_utils.cpp $(SRC_DIR)/physics_utils.h
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/test_physics.cpp $(SRC_DIR)/physics_utils.cpp -o $@ $(LDFLAGS)

run: all
	$(TARGET)

run-test: test
	$(TEST_TARGET)

run-physics-test: $(PHYSICS_TEST_TARGET)
	$(PHYSICS_TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean test run-test run-physics-test
