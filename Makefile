# Jessica Seabolt CMP SCI 4280 Project Updated 04/13/2024

CC = gcc

CFLAGS = -Wall -Wextra -Werror -std=c99

SOURCES = main.c scanner.c parser.c stack.c

OBJECTS = $(SOURCES:.c=.o)

TARGET = statSem

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean