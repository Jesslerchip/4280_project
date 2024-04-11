# Jessica Seabolt CMP SCI 4280 Project 1 Updated 03/12/2024

CC = gcc

CFLAGS = -Wall -Wextra -Werror -std=c99

SOURCES = main.c scanner.c testScanner.c

OBJECTS = $(SOURCES:.c=.o)

TARGET = scanner

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean