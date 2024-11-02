CC = gcc
CFLAGS = -Wall -g -Iinclude
OBJ = src/execute.o src/jobs.o src/built_in.o src/parsing/parse_interface.o
TARGET = quash

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

doc:
	doxygen Doxyfile

clean:
	rm -f src/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all doc clean run
