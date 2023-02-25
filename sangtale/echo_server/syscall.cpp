#include "echo_server.hpp"

/**
 * @brief 소켓 디스크립터(SD)를 반환 and 시스템 콜 에러 핸들링
 * @return socket()의 return value, SD(FD)
 */
int __socket_init() {
  const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1)
    exit_with_error("socket() error\n" + std::string(strerror(errno)));
  return (server_socket);
}

/**
 * @brief 서버 어드레스를 초기화
 * @param server_addr
 */
void __init_server_addr(struct sockaddr_in &server_addr) {
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(8080);
}

/**
 * @brief 소켓에 IP 주소와 포트 번호를 할당
 * @param sd, server_addr
 */
void __bind_handling(int server_socket, struct sockaddr_in &server_addr) {
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1)
    exit_with_error("bind() error\n" + std::string(strerror(errno)));
}

/**
 * @brief 서버 소켓에서 클라이언트의 접속을 대기
 *        wait queue는 대기 큐로, 크기는 5
 * @param sd
 */
void __listen_handling(int sd) {
  const int count_wait_queue = 5;
  if (listen(sd, count_wait_queue) == -1)
    exit_with_error("listen() error\n" + std::string(strerror(errno)));
  fcntl(sd, F_SETFL, O_NONBLOCK);
}

int __kqueue_handling() {
  const int kq = kqueue();
  if (kq == -1)
    exit_with_error("kqueue() error\n" + std::string(strerror(errno)));
  return (kq);
}