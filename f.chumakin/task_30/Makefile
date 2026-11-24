CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Wextra
LDLIBS = -lsocket -lnsl
TARGETS = server client

all: $(TARGETS)

server: server.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o server server.c $(LDLIBS)

client: client.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o client client.c $(LDLIBS)

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
