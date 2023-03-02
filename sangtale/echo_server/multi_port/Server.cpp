#include "Server.hpp"
#include "Syscall.hpp"

Server::Server(int argc, char **av) {
  if (argc == 1)
    exit_with_error("Usage\n $> ./multi_port_serv {port N}");
}

void Server::changeEvents(std::vector<struct kevent> &changeList_,
                          uintptr_t ident, int16_t filter, uint16_t flags,
                          uint16_t fflags, intptr_t data, void *udata) {
  struct kevent tempEvent;

  EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
  changeList_.push_back(tempEvent);
}

void Server::disconnectClients(int clientFd,
                               std::map<int, std::string> &clients_) {
  std::cout << "client disconnected : " << clientFd << std::endl;
  close(clientFd);
  clients_.erase(clientFd);
}
