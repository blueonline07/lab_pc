CXX = /opt/homebrew/bin/g++-15
CXXFLAGS = -O3 -fopenmp -march=native -Wall -std=c++17
LDFLAGS =
CPPFLAGS =
TARGET = build/heat_diffusion
SRCDIR = src
BUILDDIR = build
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/heat_diffusion.cpp $(SRCDIR)/utils.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

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
	@echo "  all     - Build the executable (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  run     - Build and run the program"
	@echo "  debug   - Build with debug symbols"
	@echo "  help    - Show this help message"
