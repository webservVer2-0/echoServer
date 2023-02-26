CC = c++
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread

.PHONY: re clean client echoServer

all: echoServer client

echoServer: main.o syscall.o
	$(CC) $(CFLAGS) $^ -o $@

client: client.cpp
	$(CC) $(CFLAGS) $< -o $@

main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

syscall.o: syscall.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f echoServer client *.o
