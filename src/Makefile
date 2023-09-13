# Compiler settings
CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -fomit-frame-pointer -ffast-math -flto -funroll-loops -finline-functions -fno-rtti


# Source files
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.hpp)

# Output executable
EXE = chess_engine

.PHONY: all clean

all: $(EXE)

$(EXE): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

clean:
	rm -f $(EXE)