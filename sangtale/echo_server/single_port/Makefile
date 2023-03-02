CC = c++
LDFLAGS = -lpthread

all: echoServer client
	@echo "Done."

echoServer: main.o syscall.o
	@echo "echoServer Compiling..."
	@$(CC) $^ -o $@

client: client.cpp
	@echo "client Compiling..."
	@$(CC) $< -o $@

main.o: main.cpp
	@$(CC) -c $< -o $@

syscall.o: syscall.cpp
	@$(CC) -c $< -o $@

clean:
	@rm -f echoServer client *.o