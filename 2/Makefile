# === Compiler settings ===
CXX       = g++
MPICXX    = mpic++
CXXFLAGS  = -O3 -std=c++17 -Wall -Wextra
LDFLAGS   = 

# === Project setup ===
SRC_DIR = src
BUILD_DIR = build
SIMULATION_OBJ = $(SRC_DIR)/simulation.cpp

# === Targets ===
SEQUENTIAL_TARGET = $(BUILD_DIR)/sequential
PARALLEL_TARGET = $(BUILD_DIR)/parallel
TEST_TARGET = $(BUILD_DIR)/test_validation

# === Rules ===
all: $(SEQUENTIAL_TARGET) $(PARALLEL_TARGET)

sequential: $(SEQUENTIAL_TARGET)

parallel: $(PARALLEL_TARGET)

$(SEQUENTIAL_TARGET): $(SRC_DIR)/sequential.cpp $(SIMULATION_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(PARALLEL_TARGET): $(SRC_DIR)/parallel.cpp $(SIMULATION_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(MPICXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_TARGET): $(SRC_DIR)/test_validation.cpp $(SIMULATION_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(MPICXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# === Run targets ===
run-sequential: sequential
	./$(SEQUENTIAL_TARGET)

run-parallel: parallel
	mpirun -np 2 $(PARALLEL_TARGET)

run-parallel-2: parallel
	mpirun -np 2 $(PARALLEL_TARGET)

run-parallel-4: parallel
	mpirun -np 4 $(PARALLEL_TARGET)

run-parallel-8: parallel
	mpirun -np 8 $(PARALLEL_TARGET)

# === Test targets ===
test: validate

validate: $(TEST_TARGET)
	mpirun -np 2 $(TEST_TARGET)

test-4: $(TEST_TARGET)
	mpirun -np 4 $(TEST_TARGET)

benchmark: all
	@echo "=== Performance Benchmark ==="
	@echo "Sequential version:"
	@time ./$(SEQUENTIAL_TARGET) > /dev/null
	@echo ""
	@echo "Parallel version (2 processes):"
	@time mpirun -np 2 $(PARALLEL_TARGET) > /dev/null
	@echo ""
	@echo "Parallel version (4 processes):"
	@time mpirun -np 4 $(PARALLEL_TARGET) > /dev/null

# === Clean ===
clean:
	rm -rf $(BUILD_DIR)

# === Help ===
help:
	@echo "Available targets:"
	@echo "  all              - Build both sequential and parallel versions"
	@echo "  sequential       - Build only sequential version"
	@echo "  parallel         - Build only parallel version"
	@echo "  test             - Run validation test with 2 processes"
	@echo "  validate         - Run validation test with 2 processes"
	@echo "  test-4           - Run validation test with 4 processes"
	@echo "  run-sequential   - Run sequential simulation"
	@echo "  run-parallel     - Run parallel simulation with 2 processes"
	@echo "  run-parallel-2   - Run parallel simulation with 2 processes"
	@echo "  run-parallel-4   - Run parallel simulation with 4 processes"
	@echo "  run-parallel-8   - Run parallel simulation with 8 processes"
	@echo "  benchmark        - Performance comparison"
	@echo "  clean            - Remove build directory"
	@echo "  help             - Show this help message"

.PHONY: all sequential parallel run-sequential run-parallel run-parallel-2 run-parallel-4 run-parallel-8 test validate test-4 benchmark clean help
