#include "echo_server.hpp"

static void error_handling(const char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

int main(int ac, char *av[]) {
  int sock;
  struct sockaddr_in serv_addr;
  char message[100];

  if (ac != 3) {
    printf("argument error\n");
    printf("usage : ./client 127.0.0.1 8080");
    exit(EXIT_FAILURE);
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    error_handling("socket() error");
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(av[1]);
  serv_addr.sin_port = htons(atoi(av[2]));

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    error_handling("connect() error.");
  }

  int n;
  n = scanf("%[^\n]s", message);
  if (n == -1) {
    error_handling("scanf() error");
  }
  n = write(sock, message, sizeof(message));
  if (n == -1) {
    error_handling("write() error");
  }
  close(sock);
  return (0);
}
