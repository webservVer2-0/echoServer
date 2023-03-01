#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#define BUFSIZE 1024

void error_handle(const std::string& error_msg) {
  std::cerr << error_msg << std::endl;
  exit(1);
}

void change_event(std::vector<struct kevent>& changelist, uintptr_t ident,
                  int16_t filter, uint16_t flags, uint32_t fflags,
                  intptr_t data, void* udata) {
  struct kevent tmp_ev;

  EV_SET(&tmp_ev, ident, filter, flags, fflags, data, udata);
  changelist.push_back(tmp_ev);
}

void disconnect_client(int client_fd, std::map<int, std::string>& clients) {
  std::cout << "client disconnected : " << client_fd << std::endl;
  close(client_fd);
  clients.erase(client_fd);
}

int main(int argc, char** argv) {
  int s_sock;
  int c_sock;
  char msg[BUFSIZE];

  struct sockaddr_in sv_addr;

  if (argc != 2) {
    std::cout << "usage : " << argv[0] << "port\n ";
    exit(1);
  }

  if ((s_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    error_handle("socket() error");

  memset(&sv_addr, 0, sizeof(sv_addr));
  sv_addr.sin_family = AF_INET;
  sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(s_sock, (struct sockaddr*)&sv_addr, sizeof(sv_addr)) == -1)
    error_handle("bind() error");

  if (listen(s_sock, 5) == -1) error_handle("listen() error");

  fcntl(s_sock, F_SETFL, O_NONBLOCK);

  // init kqueue
  int kq;
  if ((kq = kqueue()) == -1) error_handle("kqueue() error");

  std::map<int, std::string> clients;     // map for client socketdata
  std::vector<struct kevent> changelist;  // kevent vector for changelist
  struct kevent eventlist[8];             // kevent array for eventlist

  // add event for server_socket
  change_event(changelist, s_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

  std::cout << "echo server started\n";

  // kqueue
  // EV_SET(server)
  // kevent
  // accept() 후
  //  EV_SET(client)
  // if server or client
  // server -> client 생성루프

  // main loop
  int new_ev;
  struct kevent* cur_ev;
  while (1) {
    // apply changes and return new events(pending events)
    new_ev = kevent(kq, &changelist[0], changelist.size(), eventlist, 8, NULL);
    if (new_ev == -1) error_handle("kevent() error");

    changelist.clear();
    // clear changelist for new change

    for (int i = 0; i < new_ev; ++i) {
      cur_ev = &eventlist[i];

      // check error event
      if (cur_ev->flags & EV_ERROR) {
        if (cur_ev->ident == s_sock)
          error_handle("server socket error");
        else {
          std::cerr << "client socket error\n";
          disconnect_client(cur_ev->ident, clients);
        }
      } else if (cur_ev->filter == EVFILT_READ) {
        if (cur_ev->ident == s_sock) {
          // accept new client
          if ((c_sock = accept(s_sock, NULL, NULL)) == -1)
            error_handle("accept() error");

          std::cout << "accept new client : " << c_sock << std::endl;
          fcntl(c_sock, F_SETFL, O_NONBLOCK);

          // add event for client socket
          // add read & write event
          change_event(changelist, c_sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
                       0, NULL);
          change_event(changelist, c_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0,
                       0, NULL);
          clients[c_sock] = "";
        } else if (clients.find(cur_ev->ident) != clients.end()) {
          // read data from client
          int str_len = read(cur_ev->ident, msg, BUFSIZE);

          if (str_len <= 0) {
            if (str_len < 0) error_handle("client read() error");
            disconnect_client(cur_ev->ident, clients);
          } else {
            msg[str_len] = '\0';
            clients[cur_ev->ident] += msg;
            std::cout << "reveived data from " << cur_ev->ident << ": "
                      << clients[cur_ev->ident] << std::endl;
          }
        }
      } else if (cur_ev->filter == EVFILT_WRITE) {
        // send data to client
        std::map<int, std::string>::iterator it = clients.find(cur_ev->ident);
        if (it != clients.end()) {
          if (clients[cur_ev->ident] != "") {
            int str_len;
            if ((str_len = write(cur_ev->ident, clients[cur_ev->ident].c_str(),
                                 clients[cur_ev->ident].size())) == -1)
              error_handle("client write() error");
            disconnect_client(cur_ev->ident, clients);
          } else
            clients[cur_ev->ident].clear();
        }
      }
    }
  }
}

/* 앞으로 공부해야할 것 */
// GET localhost, 127.0.0.1:4242 -> /index.html open -> read -> req -> entity ->
// send
//  NONBLOCK
//  setsockopt
//  HTTP 1.1 header -> 필수 헤더 파악
//  우아한 커넥션