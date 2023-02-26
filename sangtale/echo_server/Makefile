CC = c++
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread

ifeq ($(shell uname),Linux)
    CFLAGS += -d linux
endif

.PHONY: re clean client echoServer

all: echoServer client

echoServer: main.o syscall.o
	$(CC) $(CFLAGS) $^ -o $@
	@make clean

client: client.cpp
	$(CC) $(CFLAGS) $< -o $@
	@make clean

main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

syscall.o: syscall.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f echoServer client *.o
