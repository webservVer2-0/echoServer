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

static void event_control(int server_socket, struct sockaddr_in server_addr) {
  int kq = __kqueue_handling();
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
  }
}

int main(int ac, char **av, char **en) {
  int server_socket;
  struct sockaddr_in server_addr;

  server_socket = __socket_init();
  __init_server_addr(server_addr);
  __bind_handling(server_socket, server_addr);
  __listen_handling(server_socket);

  event_control(server_socket, server_addr);
  return (EXIT_SUCCESS);
}