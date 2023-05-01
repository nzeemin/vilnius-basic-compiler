
CXXFLAGS = -std=c++17 -O3 -Wall

SOURCES = main.cpp model.cpp tokenizer.cpp parser.cpp validator.cpp generator.cpp

OBJECTS_VIBASC = main.o model.o tokenizer.o parser.o validator.o generator.o

all: vibasc

vibasc: $(OBJECTS_VIBASC)
	$(CXX) $(CXXFLAGS) -o vibasc $(OBJECTS_VIBASC)

.PHONY: clean

clean:
	rm -f $(OBJECTS_VIBASC)
