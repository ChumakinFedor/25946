CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Wextra
TARGET = pipe_upper
SRC = pipe_upper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
