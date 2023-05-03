
CXXFLAGS = -std=c++17 -O3 -Wall

SOURCES_TESTRUNNER = testrunner/testrunner.cpp
SOURCES = main.cpp model.cpp tokenizer.cpp parser.cpp validator.cpp generator.cpp $(SOURCES_TESTRUNNER)

OBJECTS_VIBASC = main.o model.o tokenizer.o parser.o validator.o generator.o
OBJECTS_TESTRUNNER = testrunner/testrunner.o

all: vibasc testrunner

vibasc: $(OBJECTS_VIBASC)
	$(CXX) $(CXXFLAGS) -o vibasc $(OBJECTS_VIBASC)

testrunner: $(OBJECTS_TESTRUNNER)
	$(CXX) $(CXXFLAGS) -o testrunner/testrunner $(OBJECTS_TESTRUNNER)

.PHONY: clean

clean:
	rm -f $(OBJECTS_VIBASC)
	rm -f $(OBJECTS_TESTRUNNER)
