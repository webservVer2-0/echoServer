#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <sys/socket.h> // socket(), bind(), listen(), accept(), connect()
#include <arpa/inet.h>

#define SYSCALL_ERROR -1
#define BACK_LOG_MAX 5

static void	error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

static void print_usage(char *av) {
	printf("Usage : %s <port>\n", av);
	exit(EXIT_FAILURE);
}

void serv_socket_init(sockaddr_in *serv_addr, char *av) {
	memset(serv_addr, 0, sizeof(serv_addr));
	serv_addr->sin_family = PF_INET;	// PF_INET == AF_INET == 2
	serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr->sin_port = htons(atoi(av));
}

int main(int ac, char *av[]) {
	int serv_sock;						// 서버 소켓 fd
	int	clnt_sock;						// 클라이언트 소켓 fd

	struct sockaddr_in serv_addr;		// 서버 정보
	struct sockaddr_in clnt_addr;		// 소켓 정보
	socklen_t	clnt_addr_size;

	if (ac != 2) {
		print_usage(av[0]);
	}

	char message[100];

	// 소켓 생성
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == SYSCALL_ERROR) {
		error_handling("socket error");
	}

	// 서버 소켓 정보 등록
	serv_socket_init(&serv_addr, av[1]);

	// 서버 소켓에 주소 정보 할당
	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SYSCALL_ERROR) {
		error_handling("bind() error");
	}

	//	서버 소켓 연결 대기 상태
	if (listen(serv_sock, BACK_LOG_MAX) == SYSCALL_ERROR) {
		error_handling("listen() error");
	}

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	// 연결 요청 대기 : 클라이언트에서 요청이 없다면 계속 block 상태
	if (clnt_sock == -1) {
		error_handling("accept() error");
	}
	
	scanf("%[^\n]s", message);
	write(clnt_sock, message, sizeof(message)); // 데이터 송신
	close(clnt_sock);
	close(serv_sock);

	return 0;
}