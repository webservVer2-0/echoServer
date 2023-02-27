#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define PORT 4242
#define BUFFER_SIZE 1024

int main() {
  // 소켓 생성 및 바인딩
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server_addr;
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);
  int erno =
      bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  std::cout << strerror(errno) << std::endl;
  int erno2 = listen(server_socket, SOMAXCONN);
  std::cout << strerror(errno) << std::endl;
  int wn = 0;

  // kqueue 생성
  int kq = kqueue();
  if (kq == -1) {
    std::cerr << "Failed to create kqueue" << std::endl;
    return -1;
  }

  // 이벤트 구조체 초기화
  struct kevent events[2];
  EV_SET(&events[0], server_socket, EVFILT_READ, EV_ADD, 0, 0, nullptr);
  int num_events = 1;

  std::cout << "Echo server started on port " << PORT << std::endl;

  while (true) {
    // 이벤트 대기
    if (wn > 3) break;
    int nev = kevent(kq, events, num_events, events, 10, nullptr);
    if (nev == -1) {
      std::cerr << "Failed to wait for events" << std::endl;
      break;
    }

    // 이벤트 처리
    for (int i = 0; i < nev; ++i) {
      if (events[i].ident == server_socket) {
        // 클라이언트 연결 요청
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(
            server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        // 클라이언트 소켓을 kqueue에 추가
        EV_SET(&events[num_events], client_socket, EVFILT_READ, EV_ADD, 0, 0,
               nullptr);
        ++num_events;

        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr)
                  << std::endl;
      } else {
        // 클라이언트 데이터 수신
        int client_socket = events[i].ident;
        char buffer[BUFFER_SIZE];
        int n = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (n <= 0) {
          // 클라이언트 연결 종료
          std::cout << "Client disconnected" << std::endl;
          EV_SET(&events[i], client_socket, EVFILT_READ, EV_DELETE, 0, 0,
                 nullptr);
          wn++;
          //   sleep(5);
        } else {
          // 클라이언트 데이터 전송
          write(1, buffer, n);
        }
      }
    }
  }

  // kqueue 종료
  close(kq);

  return 0;
}
