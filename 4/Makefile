# === Compiler settings ===
CXX = mpic++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

# === Directories ===
SRC_DIR = src
BUILD_DIR = build

# === Source files ===
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/shock_simulation.cpp \
          $(SRC_DIR)/physics_utils.cpp

OBJECTS = $(BUILD_DIR)/main.o \
          $(BUILD_DIR)/shock_simulation.o \
          $(BUILD_DIR)/physics_utils.o

TARGET = $(BUILD_DIR)/mpi_shock_simulation

# === Build rules ===
all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# === Run targets ===
# Run locally (single machine, multiple processes)
run: $(TARGET)
	mpirun -np 4 $(TARGET)

# Run with hostfile (multi-host)
run-multi: $(TARGET)
	mpirun -np 8 --hostfile hosts.txt $(TARGET)

# Run synchronous only
run-sync: $(TARGET)
	mpirun -np 4 --hostfile hosts.txt $(TARGET) --sync-only

# Run asynchronous only
run-async: $(TARGET)
	mpirun -np 4 --hostfile hosts.txt $(TARGET) --async-only

# Run without CSV output
run-no-output: $(TARGET)
	mpirun -np 4 --hostfile hosts.txt $(TARGET) --no-output

# === Clean ===
clean:
	rm -rf $(BUILD_DIR) *.csv

# === Help ===
help:
	@echo "Available targets:"
	@echo "  make           - Build the project"
	@echo "  make run       - Run locally with 4 processes"
	@echo "  make run-multi - Run with hostfile (8 processes)"
	@echo "  make run-sync  - Run synchronous version only"
	@echo "  make run-async - Run asynchronous version only"
	@echo "  make run-no-output - Run without CSV output"
	@echo "  make clean     - Remove build files and CSV outputs"
	@echo "  make help      - Show this help message"

.PHONY: all run run-multi run-sync run-async run-no-output clean help
