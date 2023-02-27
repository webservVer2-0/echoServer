#include <arpa/inet.h>
// #include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFSIZE 1024

void error_handle(char *error_msg) {
  fputs(error_msg, stderr);
  fputc('\n', stderr);
  exit(1);
}

int main(int argc, char **argv) {
  int sv_sock;
  int cl_sock;
  char msg[BUFSIZE];
  int str_len;

  struct sockaddr_in sv_addr;
  struct sockaddr_in cl_addr;
  int cl_addr_size;

  if (argc != 2) {
    printf("usage : %s port\n", argv[0]);
    exit(1);
  }

  if ((sv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    error_handle("socket() error");

  memset(&sv_addr, 0, sizeof(sv_addr));
  sv_addr.sin_family = AF_INET;
  sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(sv_sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) == -1)
    error_handle("bind() error");

  if (listen(sv_sock, 5) == -1) error_handle("listen() error");

  cl_addr_size = sizeof(cl_addr);

  if ((cl_sock = accept(sv_sock, (struct sockaddr *)&cl_addr, &cl_addr_size)) ==
      -1)
    error_handle("accept() error");

  while ((str_len = read(cl_sock, msg, BUFSIZE)) != 0) {
    write(1, msg, str_len);
    write(cl_sock, msg, str_len);
  }

  close(cl_sock);
}
