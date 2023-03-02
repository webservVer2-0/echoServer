#pragma ONCE

#include "syscall.hpp"

class Server {
public:
  Server(int argc, char **argv);
  ~Server();
  void changeEvents(std::vector<struct kevent> &changeList_, uintptr_t ident,
                    int16_t filter, uint16_t flags, uint16_t fflags,
                    intptr_t data, void *udata);
  void disconnectClients(int clientFd, std::map<int, std::string> &clients_);

private:
  int *server_socket;
};