# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall -Wno-unknown-pragmas -Wno-unused-label

# Source files
CXXSOURCES = HashTable.cpp main.cpp

# Header files
HEADERS = table_dt.hpp utils.hpp tpch_read_3.hpp prepare.hpp HashTable.hpp

# Object files
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)

# Executable name
EXECUTABLE = main

# Targets and rules
.PHONY: all clean help

all: $(EXECUTABLE)

$(EXECUTABLE): $(CXXOBJECTS)
	$(CXX) $(CFLAGS) $(CXXOBJECTS) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(CXXOBJECTS) $(EXECUTABLE)

help:
	@echo " Available targets:"
	@echo "  all     : Build the executable (default target)"
	@echo "  clean   : Remove object files and the executable"
	@echo "  help    : Show this help message"
