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

//multi port echo server 구현

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

// bool  is_server(uintptr_t cur_ident)
// {
//   for (int i = 0; i < port_cnt; i++)
//   {
//     if (cur_ident == )
//   }
// }

int main(int argc, char** argv) {
  int port_cnt;
  int* s_socks;
  struct sockaddr_in* sv_addrs;
  int c_sock;
  char msg[BUFSIZE];

  port_cnt = argc - 1;
  if (port_cnt <= 0) {
    std::cout << "usage : " << argv[0] << " port1 (port2 ... portn)\n";
    exit(1);
  }
  s_socks = new int[port_cnt];//new() error 처리할말?
  sv_addrs = new sockaddr_in[port_cnt];
  for (int i = 0; i < port_cnt; i++)
  {
    if ((s_socks[i] = socket(PF_INET, SOCK_STREAM, 0)) == -1)
      error_handle("socket() error");
    memset(&sv_addrs[i], 0, sizeof(sv_addrs[i]));
    sv_addrs[i].sin_family = AF_INET;
    sv_addrs[i].sin_addr.s_addr = htonl(INADDR_ANY);
    sv_addrs[i].sin_port = htons(atoi(argv[1 + i]));
  }
  
  for (int i = 0; i < port_cnt; i++)
  {
    if (bind(s_socks[i], (struct sockaddr*)(&sv_addrs[i]), sizeof(sv_addrs[i])) == -1)
      error_handle("bind() error");
  }

  for (int i = 0; i < port_cnt; i++)
  {
    std::cout << "server's socket" << i + 1 << " descriptor : " << s_socks[i] << std::endl;
    std::cout << "server's socket" << i + 1 << " port : " << ntohs(sv_addrs[i].sin_port) << std::endl;
  }

  for (int i = 0; i < port_cnt; i++)
  {
    if (listen(s_socks[i], 4) == -1) error_handle("listen() error");
    fcntl(s_socks[i], F_SETFL, O_NONBLOCK);
  }

  // init kqueue
  int kq;
  if ((kq = kqueue()) == -1) error_handle("kqueue() error");

  std::map<int, std::string> clients;     // map for client socketdata
  std::vector<struct kevent> changelist;  // kevent vector for changelist
  struct kevent eventlist[42];             // kevent array for eventlist

  // add event for server_socket
  for (int i = 0; i < port_cnt; i++)
  {
    change_event(changelist, s_socks[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    std::cout << "echo server" << i + 1 << " started\n";
  }

  // main loop
  int ev_cnt_in_evlst = 0;
  struct kevent* cur_ev;
  while (1) {
    // apply changes and return new events(pending events)
    ev_cnt_in_evlst = kevent(kq, &changelist[0], changelist.size(), eventlist, 42, NULL);
    if (ev_cnt_in_evlst == -1) error_handle("kevent() error");

    changelist.clear();
    // clear changelist for new change

    for (int i = 0; i < ev_cnt_in_evlst; ++i) {
      cur_ev = &eventlist[i];

      // check error event
      if (cur_ev->flags & EV_ERROR) {
        for (int i = 0; i < port_cnt; i++)
        {
          if (cur_ev->ident == s_socks[i])
            error_handle("server socket error");
          else {
            std::cerr << "client socket error\n";
            disconnect_client(cur_ev->ident, clients);
          }
        }
      } else if (cur_ev->filter == EVFILT_READ) {
        int is_server = 0;
        for (int i = 0; i < port_cnt; i++)
        {
          if (cur_ev->ident == s_socks[i])
          {
            is_server++;
            break;
          }
        }
        if (is_server)
        {
          // accept new client
          if ((c_sock = accept(cur_ev->ident, NULL, NULL)) == -1)
            error_handle("accept() error");
          std::cout << "accept new client : " << c_sock << std::endl;
          // accept된 소켓의 디스크립터
          fcntl(c_sock, F_SETFL, O_NONBLOCK);

          // add event for client socket
          // add read & write event
          change_event(changelist, c_sock, EVFILT_READ, EV_ADD | EV_ENABLE
                       | EV_EOF, 0, 0, NULL);
          change_event(changelist, c_sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0,
                       0, NULL);
          clients[c_sock] = "";
        }
        else if (clients.find(cur_ev->ident) != clients.end()) {
          // read data from client
          int str_len = read(cur_ev->ident, msg, BUFSIZE);

          if (str_len <= 0) {
            if (str_len < 0) error_handle("client read() error");
            //필요시 clear()
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
            // if ((str_len = write(cur_ev->ident, clients[cur_ev->ident].c_str(),
            //                      clients[cur_ev->ident].size())) == -1)
            if ((str_len = write(cur_ev->ident, "HTTP/1.1 200 OK\r\nContent-Length:2\r\n\r\nhi",
                                 strlen("HTTP/1.1 200 OK\r\nContent-Length:2\r\n\r\nhi"))) == -1)
              error_handle("client write() error");
            disconnect_client(cur_ev->ident, clients);
          } else
              clients[cur_ev->ident].clear();
              // disconnect_client(cur_ev->ident, clients);
        }
      }
    }
  }
  delete[] s_socks;
  delete[] sv_addrs;
}
  // kqueue
  // EV_SET(server)
  // kevent
  // accept() 후
  //  EV_SET(client)
  // if server or client
  // server -> client 생성루프

/* 앞으로 공부해야할 것 */
// GET localhost, 127.0.0.1:4242 -> /index.html open -> read -> req -> entity ->
// send
//  NONBLOCK
//  setsockopt
//  HTTP 1.1 header -> 필수 헤더 파악
//  우아한 커넥션