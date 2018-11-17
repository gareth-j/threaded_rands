CC = g++
CFLAGS = -c -O3 -fopenmp -std=c++17
LDFLAGS= -fopenmp
SOURCES = main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=threaded

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o



