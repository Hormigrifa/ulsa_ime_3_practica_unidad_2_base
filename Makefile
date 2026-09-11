CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC := $(wildcard src/*.cpp)
OUT = ./build/trafico

all: $(OUT)

$(OUT): $(SRC)
	@mkdir -p $(dir $(OUT))
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT) $(OUT).exe

.PHONY: all run clean