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
static void change_events(std::vector<struct kevent> &change_list,
                          uintptr_t ident, int16_t filter, uint16_t flags,
                          uint32_t fflags, intptr_t data, void *udata) {
  struct kevent temp_event;

  EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
  change_list.push_back(temp_event);
}

static void disconnect_client(int client_fd,
                              std::map<int, std::string> &clients) {
  std::cout << "client disconnected: " << client_fd << "\n";
  close(client_fd);
  clients.erase(client_fd);
}

static void kevent_control(int server_socket) {
  const int kq = __kqueue_handling();
  std::map<int, std::string> clients;     // map for client socket:data
  std::vector<struct kevent> change_list; // kevent vector for changelist
  struct kevent event_list[8];            // kevent array for eventlist

  /* add event for server socket */
  change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0,
                0, NULL);
  std::cout << "echo server started.\n";

  int new_events;
  struct kevent *curr_event;
  while (true) {
    new_events =
        kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
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
      } else if (curr_event->filter == EVFILT_READ) {
        if ((int)curr_event->ident == server_socket) {
          /* accept new client */
          printf("read event 1 : %d\n", new_events);
          int client_socket = __accept_handling(server_socket);
          std::cout << "accept new client : " << client_socket << "\n";
          __fcntl_handling(client_socket);

          /* add event for client socket - add read && write event */
          change_events(change_list, client_socket, EVFILT_READ,
                        EV_ADD | EV_ENABLE, 0, 0, NULL);
          change_events(change_list, client_socket, EVFILT_WRITE,
                        EV_ADD | EV_ENABLE, 0, 0, NULL);
          clients[client_socket] = "";
        } else if (clients.find(curr_event->ident) != clients.end()) {
          /* read data from client */
          char buf[1024];
          int n = read(curr_event->ident, buf, sizeof(buf));
          if (n <= 0) {
            if (n < 0)
              std::cerr << "client read error\n";
            disconnect_client(curr_event->ident, clients);
          } else {
            buf[n] = '\0';
            clients[curr_event->ident] += buf;
            std::cout << "received data from " << curr_event->ident << ": "
                      << clients[curr_event->ident] << "\n";
          }
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