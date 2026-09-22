CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2
TARGET=flight_simulator
SRC=src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f logs/flight.log

.PHONY: all run clean
