CXX = mpic++
CXXFLAGS = -Wall -O2
SRCDIR ?= src/2
SOURCES = $(SRCDIR)/$(TARGET).cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Handle common.cpp if it exists in the source directory
ifneq ($(wildcard $(SRCDIR)/common.cpp),)
    COMMON_OBJ = $(SRCDIR)/common.o
    OBJECTS += $(COMMON_OBJ)
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean

