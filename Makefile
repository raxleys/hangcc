.PHONY: all run build clean

CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -ggdb
EXEC=build/hangcc

all: build

run: build
	./$(EXEC)

build:
	mkdir -p build
	$(CXX) -o $(EXEC) src/hangcc.cc -Iinc $(CXXFLAGS)

clean:
	rm -rf build
