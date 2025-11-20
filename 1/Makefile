CXX = /opt/homebrew/bin/g++-15
CXXFLAGS = -O3 -fopenmp -march=native -Wall -std=c++17
LDFLAGS =
CPPFLAGS =
TARGET = build/heat_diffusion
TEST_TARGET = build/test_validation
SRCDIR = src
BUILDDIR = build
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/heat_diffusion.cpp $(SRCDIR)/utils.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
TEST_SOURCES = $(SRCDIR)/test_validation.cpp $(SRCDIR)/heat_diffusion.cpp $(SRCDIR)/utils.cpp
TEST_OBJECTS = $(TEST_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

.PHONY: all clean run test validate

all: $(TARGET)

test: validate

validate: $(TEST_TARGET)
	$(TEST_TARGET)

$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

$(TEST_TARGET): $(TEST_OBJECTS) | $(BUILDDIR)
	$(CXX) $(TEST_OBJECTS) -o $(TEST_TARGET) $(CXXFLAGS) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

run: $(TARGET)
	cd $(BUILDDIR) && ./heat_diffusion

# Debug build
debug: CXXFLAGS = -g -fopenmp -Wall -std=c++17
debug: $(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the executable (default)"
	@echo "  test     - Run validation test"
	@echo "  validate - Run validation test"
	@echo "  clean    - Remove build artifacts"
	@echo "  run      - Build and run the program"
	@echo "  debug    - Build with debug symbols"
	@echo "  help     - Show this help message"
