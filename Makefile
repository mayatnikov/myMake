CC = g++
CFLAGS = -c -Wall -std=c++11 -g
LDFLAGS =
SOURCES = main.cpp target.cpp stringProcessor.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = mymake

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
