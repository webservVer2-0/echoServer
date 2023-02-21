#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <sys/socket.h> // socket(), bind(), listen(), accept(), connect()
#include <arpa/inet.h>

static void	error_handling(char message[]) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

int main(int ac, char *av[]) {
	int		sock;
	int		str_len;
	struct 	sockaddr_in	serv_addr;
	char	message[100];
	
	if (ac != 3) {
		printf("argument error\n");
		exit(EXIT_FAILURE);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		error_handling("socket() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(av[1]);
	serv_addr.sin_port = htons(atoi(av[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		error_handling("connect() error.");
	}

	str_len = read(sock, message, sizeof(message) - 1);
	if (str_len == -1) {
		error_handling("read() error");
	}

	printf("Message from server : %s\n", message);
	close(sock);
	return 0;
}