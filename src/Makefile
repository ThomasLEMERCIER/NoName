# Compiler settings
CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -fomit-frame-pointer -ffast-math -flto -funroll-loops -finline-functions -fno-rtti


# Source files
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.hpp)

# Output executable
EXECUTABLE = chess_engine.exe

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

clean:
	rm -f $(EXECUTABLE)