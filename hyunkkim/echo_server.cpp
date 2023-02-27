#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

void exit_with_perror(const std::string& msg) {
  std::cerr << msg << std::endl;
  exit(EXIT_FAILURE);
}

void change_events(std::vector<struct kevent>& change_list, uintptr_t ident,
                   int16_t filter, uint16_t flags, uint32_t fflags,
                   intptr_t data, void* udata) {
  struct kevent tmp_event;
  EV_SET(&tmp_event, ident, filter, flags, fflags, data, udata);
  change_list.push_back(tmp_event);
}

void disconnect_client(int client_fd, std::map<int, std::string>& clients) {
  std::cout << "클라이언트가 접속을 종료함: " << client_fd << std::endl;
  close(client_fd);
  clients.erase(client_fd);
}

struct sockaddr_in& init_socket(int& server_socket,
                                struct sockaddr_in& server_addr) {
  if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    // 소켓 만들기를 시도함. 실패시 -1 리턴 -> 이때 exit함
    exit_with_perror("socket() error: 소켓 생성 실패 - " +
                     std::string(strerror(errno)));
  }
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(8080);
  return server_addr;
}

int main() {
  int server_socket;
  struct sockaddr_in server_addr;  // 소켓 주소에 대한 구조체?

  init_socket(server_socket, server_addr);

  if (bind(server_socket, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) == -1) {
    // bind를 열린 소켓에 시도하는데 그게 실패하면 -1 -> 이떄도 exit;
    exit_with_perror("bind() error: 바인드 실패 - " +
                     std::string(strerror(errno)));
  }

  // 이제 listen 시도.
  if (listen(server_socket, 5) == -1) {
    exit_with_perror("listen() error: 리슨 실패 - " +
                     std::string(strerror(errno)));
  }

  fcntl(server_socket, F_SETFL, O_NONBLOCK);

  // init kqueue
  int kq = kqueue();

  if (kq == -1) {
    exit_with_perror("kqueue() error: " + std::string(strerror(errno)));
  }

  std::map<int, std::string>
      clients;  // 클라이언트의 <socket, data>를 저장하기 위한 맵;
  std::vector<struct kevent> change_list;  // changelist를 위한 kevent vector;
  struct kevent event_list[8];             // eventlist를 위한 kevent 배열;

  // 서버 소켓을 위한 이벤트를 추가;
  change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
                0, NULL);
  std::cout << "에코 서버 시작됨" << std::endl;

  // 메인 반복문
  int new_events;
  struct kevent* curr_event;

  while (true) {
    new_events =
        kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
    if (new_events == -1) {
      exit_with_perror("kevent() error: " + std::string(strerror(errno)));
    }

    for (int i = 0; i < new_events; i++) {
      curr_event = &event_list[i];

      // 에러 체크;
      if (curr_event->flags & EV_ERROR) {
        std::cout << "에러 감지 스코프 진입" << std::endl;
        if (curr_event->ident == (uintptr_t)server_socket) {
          exit_with_perror("서버 소켓 에러");
        } else {
          std::cerr << "클라이언트 소켓 에러" << std::endl;
          disconnect_client(curr_event->ident, clients);
        }
      } else if (curr_event->filter == EVFILT_WRITE) {
        std::map<int, std::string>::iterator it =
            clients.find(curr_event->ident);
        if (it != clients.end()) {
          if (clients[curr_event->ident] != "") {
            std::cout << "클라이언트에게 data를 쓰는 중..." << std::endl;
            int n = write(curr_event->ident, clients[curr_event->ident].c_str(),
                          clients[curr_event->ident].size());
            std::cout << "[전송 완료!]" << std::endl;

            if (n == -1) {
              std::cerr << "클라이언트 write 에러!" << std::endl;
              disconnect_client(curr_event->ident, clients);
            } else {
              clients[curr_event->ident].clear();
            }
          }
        }
      } else if (curr_event->filter == EVFILT_READ) {
        if (curr_event->ident == (uintptr_t)server_socket) {
          int client_socket = accept(server_socket, NULL, NULL);

          if (client_socket == -1) {
            exit_with_perror("accept() 에러: " + std::string(strerror(errno)));
          }
          std::cout << "새로운 클라이언트를 accept: " << client_socket
                    << std::endl;
          fcntl(client_socket, F_SETFL, O_NONBLOCK);

          change_events(change_list, client_socket, EVFILT_READ,
                        EV_ADD | EV_ENABLE, 0, 0, NULL);
          change_events(change_list, client_socket, EVFILT_WRITE,
                        EV_ADD | EV_ENABLE, 0, 0, NULL);
          clients[client_socket] = "";
        } else if (clients.find(curr_event->ident) != clients.end()) {
          char buf[1024];
          std::cout << "클라이언트로 부터 data를 읽는 중..." << std::endl;
          int n = read(curr_event->ident, buf, sizeof(buf));

          if (n <= 0) {
            if (n < 0) {
              std::cerr << "클라이언트 READ 에러!!" << std::endl;
            }
            disconnect_client(curr_event->ident, clients);
          } else {
            buf[n] = '\0';  // nul문자 종료;
            clients[curr_event->ident] += buf;
            std::cout << curr_event->ident
                      << "로 부터 받은 data: " << clients[curr_event->ident]
                      << std::endl;
          }
        }
      }
    }
  }
  return (0);
}
