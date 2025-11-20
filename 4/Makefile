# === Compiler settings ===
MPICXX    = mpic++
CXXFLAGS  = -O2 -std=c++17 -Wall -Wextra
LDFLAGS   = 

# === Project setup ===
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/mpi_multi_host

# === Targets ===
all: $(TARGET)

$(TARGET): $(SRC_DIR)/main.cpp
	@mkdir -p $(BUILD_DIR)
	$(MPICXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# === Run targets ===
# Run on single host (local)
run: $(TARGET)
	mpirun -np 4 $(TARGET)

# Run on multiple hosts using hosts.txt
run-multi: $(TARGET)
	mpirun --hostfile hosts.txt -np 4 $(TARGET)

# Run with specific number of processes
run-2: $(TARGET)
	mpirun --hostfile hosts.txt -np 2 $(TARGET)

run-4: $(TARGET)
	mpirun --hostfile hosts.txt -np 4 $(TARGET)

run-8: $(TARGET)
	mpirun --hostfile hosts.txt -np 8 $(TARGET)

# === Docker targets ===
docker-up:
	docker compose up -d

docker-down:
	docker compose down

docker-build: docker-up
	@echo "Setting up SSH for multi-host communication..."
	@./setup-ssh.sh || echo "SSH setup may have issues, but continuing..."
	@echo "Building MPI program in containers..."
	docker compose exec mpi-host-1 make
	docker compose exec mpi-host-2 make

docker-run: docker-build
	@echo "Running MPI program across multiple hosts..."
	docker compose exec mpi-host-1 mpirun --allow-run-as-root --hostfile hosts.txt -np 4 $(TARGET)

docker-run-2: docker-build
	docker compose exec mpi-host-1 mpirun --allow-run-as-root --hostfile hosts.txt -np 2 $(TARGET)

docker-run-4: docker-build
	docker compose exec mpi-host-1 mpirun --allow-run-as-root --hostfile hosts.txt -np 4 $(TARGET)

docker-shell-1:
	docker compose exec mpi-host-1 /bin/bash

docker-shell-2:
	docker compose exec mpi-host-2 /bin/bash

# === Clean ===
clean:
	rm -rf $(BUILD_DIR)

docker-clean: docker-down
	rm -rf $(BUILD_DIR)

# === Help ===
help:
	@echo "Available targets:"
	@echo "  all              - Build MPI program"
	@echo "  run              - Run on single host (4 processes)"
	@echo "  run-multi        - Run on multiple hosts using hosts.txt (4 processes)"
	@echo "  run-2            - Run on multiple hosts (2 processes)"
	@echo "  run-4            - Run on multiple hosts (4 processes)"
	@echo "  run-8            - Run on multiple hosts (8 processes)"
	@echo ""
	@echo "Docker targets:"
	@echo "  docker-up        - Start Docker containers"
	@echo "  docker-down      - Stop Docker containers"
	@echo "  docker-build     - Build MPI program in containers"
	@echo "  docker-run       - Run MPI program across Docker hosts (4 processes)"
	@echo "  docker-run-2    - Run MPI program across Docker hosts (2 processes)"
	@echo "  docker-run-4    - Run MPI program across Docker hosts (4 processes)"
	@echo "  docker-shell-1   - Open shell in mpi-host-1 container"
	@echo "  docker-shell-2   - Open shell in mpi-host-2 container"
	@echo "  docker-clean     - Stop containers and clean build files"
	@echo ""
	@echo "  clean            - Remove build directory"
	@echo "  help             - Show this help message"

.PHONY: all run run-multi run-2 run-4 run-8 docker-up docker-down docker-build docker-run docker-run-2 docker-run-4 docker-shell-1 docker-shell-2 docker-clean clean help

