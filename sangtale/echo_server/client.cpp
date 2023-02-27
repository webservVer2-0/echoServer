#include "echo_server.hpp"

static void error_handling(const char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

inline int ___kqueue_handling() {
  const int kq = kqueue();
  if (kq == -1)
    error_handling("kqueue() error\n");
  return (kq);
}

static sockaddr_in __serv_addr_init(char **av) {
  struct sockaddr_in serv_addr;

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(av[1]);
  serv_addr.sin_port = htons(atoi(av[2]));
  return (serv_addr);
}

static void send_message(int sock) {
  char message[1024];

  const int sb = scanf("%[^\n]s", message);
  if (sb == -1)
    error_handling("scanf() error");
  const int wb = write(sock, message, sizeof(message));
  if (wb == -1)
    error_handling("write() error");
}

static void receive_message(int sock) {
  char buf[1024];
  const int rb = read(sock, buf, sizeof(buf));
  if (rb == -1)
    error_handling("read() error");
  buf[rb] = '\0';
  write(1, buf, rb);
}

static void kq_handling(int sock) {
  struct kevent event;
  int kq, nevent;

  /* Create a new kernel event queue */
  kq = ___kqueue_handling();
  /* Register the standard input file descriptor with the kqueue */
  EV_SET(&event, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
  nevent = kevent(kq, &event, 1, NULL, 0, NULL);
  if (nevent == -1)
    error_handling("kevent() error");
  for (int i = 0; i < nevent; ++i) {
    if (event.flags & EV_ERROR) {
      if ((int)event.ident == sock)
        error_handling("client socket error");
      else {
        std::cerr << "server socket error\n";
        close(sock);
      }
    } else if (event.filter == EVFILT_READ) {
      receive_message(sock);
    }
  }
}

int main(int ac, char *av[]) {
  if (ac != 3) {
    printf("argument error\n");
    printf("usage : ./client 127.0.0.1 8080");
    exit(EXIT_FAILURE);
  }
  const struct sockaddr_in serv_addr = __serv_addr_init(av);
  const int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
    error_handling("socket() error");
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect() error.");
  send_message(sock);
  kq_handling(sock);
  close(sock);
  return (EXIT_SUCCESS);
}
