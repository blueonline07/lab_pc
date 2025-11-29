CXX = mpic++
CXXFLAGS = -Wall -O2 -std=c++11
TARGET = bin/test_mpi
SRC = src/test_mpi.cpp
OBJ = bin/obj/test_mpi.o

# Create directories if they don't exist
$(shell mkdir -p bin bin/obj)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -rf bin/

run-local:
	mpirun -np 4 ./$(TARGET)

run-distributed:
	@if [ ! -f hosts.txt ]; then \
		echo "Error: hosts.txt not found. Please create it first."; \
		exit 1; \
	fi
	mpirun -np $$(grep -v '^#' hosts.txt | grep -v '^$$' | wc -l | tr -d ' ') --hostfile hosts.txt ./$(TARGET)

run-distributed-one-per-node:
	@if [ ! -f hosts.txt ]; then \
		echo "Error: hosts.txt not found. Please create it first."; \
		exit 1; \
	fi
	mpirun -np $$(grep -v '^#' hosts.txt | grep -v '^$$' | wc -l | tr -d ' ') --hostfile hosts.txt --map-by ppr:1:node ./$(TARGET)

.PHONY: all clean run-local run-distributed run-distributed-one-per-node

