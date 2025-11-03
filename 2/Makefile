# === Compiler settings ===
CXX       = g++
MPICXX    = mpic++
CXXFLAGS  = -O3 -std=c++17 -Wall -Wextra
LDFLAGS   = 

# === Project setup ===
SRC_DIR = src
BUILD_DIR = build
SIMULATION_OBJ = $(SRC_DIR)/simulation.cpp $(SRC_DIR)/utils.cpp

# === Targets ===
SEQUENTIAL_TARGET = $(BUILD_DIR)/sequential
PARALLEL_TARGET = $(BUILD_DIR)/parallel

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

# === Run targets ===
run-sequential: sequential
	@echo "Running sequential simulation..."
	./$(SEQUENTIAL_TARGET)

run-parallel: parallel
	@echo "Running parallel simulation with 2 processes..."
	mpirun -np 2 $(PARALLEL_TARGET)

run-parallel-2: parallel
	@echo "Running parallel simulation with 2 processes..."
	mpirun -np 2 $(PARALLEL_TARGET)

run-parallel-4: parallel
	@echo "Running parallel simulation with 4 processes..."
	mpirun -np 4 $(PARALLEL_TARGET)

run-parallel-8: parallel
	@echo "Running parallel simulation with 8 processes..."
	mpirun -np 8 $(PARALLEL_TARGET)

# === Test targets ===
test: all
	@echo "=== Running Sequential Test ==="
	@time ./$(SEQUENTIAL_TARGET)
	@echo ""
	@echo "=== Running Parallel Test (2 processes) ==="
	@time mpirun -np 2 $(PARALLEL_TARGET)

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
	@echo "  run-sequential   - Run sequential simulation"
	@echo "  run-parallel     - Run parallel simulation with 2 processes"
	@echo "  run-parallel-2   - Run parallel simulation with 2 processes"
	@echo "  run-parallel-4   - Run parallel simulation with 4 processes"
	@echo "  run-parallel-8   - Run parallel simulation with 8 processes"
	@echo "  test             - Run both versions with timing"
	@echo "  benchmark        - Performance comparison"
	@echo "  clean            - Remove build directory"
	@echo "  help             - Show this help message"

.PHONY: all sequential parallel run-sequential run-parallel run-parallel-2 run-parallel-4 run-parallel-8 test benchmark clean help
