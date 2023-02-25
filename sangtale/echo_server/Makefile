CC = c++
LDFLAGS = -lpthread

.PHONY: re clean client echoServer

all: echoServer client

echoServer: main.o syscall.o
	$(CC) $^ -o $@
	@make clean

client: client.cpp
	$(CC) $< -o $@
	@make clean

main.o: main.cpp
	$(CC) -c $< -o $@

syscall.o: syscall.cpp
	$(CC) -c $< -o $@

clean:
	rm -f echoServer client *.o
