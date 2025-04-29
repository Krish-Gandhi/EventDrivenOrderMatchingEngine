CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SOURCES = engine.cpp

TEST_DEST_PATH = build/
TEST_TARGET = a.exe
TEST_SOURCES = test.cpp engine.cpp

BINDING_TARGET = engine_py
BINDING_SOURCES = engine.cpp bindings.cpp
BINDING_DEST_PATH = ../python/

OBJS = $(SOURCES:.cpp=.o) 
EXT_SUFFIX = $(shell python3-config --extension-suffix)

all: test

test:
	$(CXX) $(CXXFLAGS) -o $(TEST_DEST_PATH)$(TEST_TARGET) $(TEST_SOURCES)
	./$(TEST_DEST_PATH)$(TEST_TARGET)

bind:
	./bind.sh

clean:
	rm -r $(TEST_DEST_PATH)
	mkdir $(TEST_DEST_PATH)
	rm -f *.o *.so *.pyd *.dll *.exe $(BINDING_TARGET)$(EXT_SUFFIX)

