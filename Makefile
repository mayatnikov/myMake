CC=g++
CCLINKER=$(CC)
CC_FLAGS=-c -Wall -std=c++11 -g
LDFLAGS=
OBJ=main.o \
 target.o \
 stringProcessor.o
EXEC=mymake

all: $(EXEC)

%.o: %.cpp
	$(CC) $(CC_FLAGS) -c $^  -o $@  

$(EXEC): $(OBJ)
	$(CCLINKER)  $(LDFLAGS) $(OBJ) -o $(EXEC)
	
clean:
	rm -f ./*.o
	rm -f ./$(EXEC)
