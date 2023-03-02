#include "echo_server.hpp"

/**
 * @brief
 *
 * @param change_list
 * @param ident               identifier for this event
 * @param filter              filter for event
 * @param flags               action flags for kqueue
 * @param fflags              filter flag value
 * @param data                filter data value
 * @param udata               opaque user data identifier
 */
void change_events(std::vector<struct kevent> &change_list, uintptr_t ident,
                   int16_t filter, uint16_t flags, uint32_t fflags,
                   intptr_t data, void *udata) {
  struct kevent temp_event;

  EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
  change_list.push_back(temp_event);
}

static void disconnect_client(int client_fd,
                              std::map<int, std::string> &clients) {
  close(client_fd);
  clients.erase(client_fd);
  std::cout << "client disconnected: " << client_fd << "\n";
}

static void kevent_control(int server_socket) {
  const int kq = __kqueue_handling();
  std::map<int, std::string> clients;     // map for client socket:data
  std::vector<struct kevent> change_list; // kevent vector for changelist
  struct kevent event_list[42];           // kevent array for eventlist

  /* add event for server socket */
  std::cout << "echo server started.\n";
  change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
                0, NULL);

  int new_events;
  struct kevent *curr_event;
  while (true) {
    const int nevents = 42;
    new_events = kevent(kq, &change_list[0], change_list.size(), event_list,
                        nevents, NULL);
    if (new_events == -1)
      exit_with_error("kevent() error\n" + std::string(strerror(errno)));
    change_list.clear(); // clear change_list for new changes

    for (int i = 0; i < new_events; ++i) {
      curr_event = &event_list[i];

      /* check error event return */
      if (curr_event->flags & EV_ERROR) {
        if ((int)curr_event->ident == server_socket)
          exit_with_error("server socket error");
        else {
          std::cerr << "client socket error\n";
          disconnect_client(curr_event->ident, clients);
        }
      }
      /* 클라이언트에게 데이터 송신 */
      else if (curr_event->filter == EVFILT_WRITE) {
        std::map<int, std::string>::iterator it =
            clients.find(curr_event->ident);
        /* 클라이언트가 있으면 */
        if (it != clients.end()) {
          /* 쓸 메시지가 있으면 */
          if (clients[curr_event->ident] != "") {
            int b = write(curr_event->ident, clients[curr_event->ident].c_str(),
                          clients[curr_event->ident].size());
            if (b == -1) {
              std::cerr << "client write() error\n";
              disconnect_client(curr_event->ident, clients);
            } else {
              clients[curr_event->ident].clear();
            }
          }
        }
      }
      /* EVFILT_READ : 읽기 가능한 데이터가 있을 때 이벤트를 감지*/
      else if (curr_event->filter == EVFILT_READ) {
        if ((int)curr_event->ident == server_socket) {
          /* accept new client */
          printf("read event 1 : %d\n", new_events);
          int client_socket = __accept_handling(server_socket);
          __fcntl_handling(client_socket);

          /* 새로운 클라이언트 소켓에 I/O 이벤트 추가 */
          change_events(change_list, client_socket, EVFILT_READ,
                        EV_ADD | EV_ENABLE, 0, 0, NULL);
          change_events(change_list, client_socket, EVFILT_WRITE,
                        EV_ADD | EV_ENABLE, 0, 0, NULL);
          clients[client_socket] = "";
        }
        /* 클라이언트가 있으면 데이터 수신 */
        else if (clients.find(curr_event->ident) != clients.end()) {
          char buf[1024];
          int n = read(curr_event->ident, buf, sizeof(buf));
          /* read error */
          if (n <= 0) {
            if (n < 0)
              std::cerr << "client read error\n";
            disconnect_client(curr_event->ident, clients);
          }
          /* 읽었으면 */
          else {
            buf[n] = '\0';
            clients[curr_event->ident] += buf;
            std::cout << "received data from " << curr_event->ident << ": "
                      << clients[curr_event->ident] << "\n";
          }
<<<<<<< HEAD:sangtale/echo_server/single_port/main.cpp
        } else if (curr_event->filter == EVFILT_WRITE) {
          /* send data to client */
          printf("write event : %d\n", new_events);
          std::map<int, std::string>::iterator it =
              clients.find(curr_event->ident);
          if (it != clients.end()) {
            if (clients[curr_event->ident] != "") {
              int n =
                  write(curr_event->ident, clients[curr_event->ident].c_str(),
                        clients[curr_event->ident].size());
              if (n == -1) {
                std::cerr << "client write() error\n";
                disconnect_client(curr_event->ident, clients);
              } else {
                clients[curr_event->ident].clear();
              }
            }
          }
=======
>>>>>>> 618d5c6f8d689c651333a1e6b11811c5995dc4c8:sangtale/echo_server/main.cpp
        }
      }
    }
  }
}

int main() {
  int server_socket;
  struct sockaddr_in server_addr;

  server_socket = __socket_init();
  __init_server_addr(server_addr);

  __bind_handling(server_socket, server_addr);

  __listen_handling(server_socket);
  __fcntl_handling(server_socket);

  kevent_control(server_socket);
  return (EXIT_SUCCESS);
}