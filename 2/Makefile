CXX       = g++-15
CXXFLAGS  = -O3 -std=c++17 -Wall -Wextra
LDFLAGS   = -lpthread
MPICXX    = /opt/homebrew/bin/mpicxx

SRC_DIR   = src
BUILD_DIR = bin
OBJ_DIR   = $(BUILD_DIR)/obj

SOURCES   = $(SRC_DIR)/simulation.cpp $(SRC_DIR)/main.cpp
OBJECTS   = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
HEADERS   = $(SRC_DIR)/simulation.h
TARGET    = $(BUILD_DIR)/main


$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(MPICXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(MPICXX) $(CXXFLAGS) -c $< -o $@

all: $(TARGET)
run: $(TARGET)
	$(TARGET) $(INPUT) $(MODE)