#include "echo_server_multi.hpp"

// private method;

void Server::exitWithPerror(const std::string& msg) {
  std::cerr << msg << std::endl;
  exit(EXIT_FAILURE);
}

void Server::changeEvents(std::vector<struct kevent>& change_list,
                          uintptr_t ident, int16_t filter, uint16_t flags,
                          uint16_t fflags, intptr_t data, void* udata) {
  struct kevent tmp_event;

  EV_SET(&tmp_event, ident, filter, flags, fflags, data, udata);
  change_list.push_back(tmp_event);
}

void Server::disconnectClients(int client_fd,
                               std::map<int, std::string>& clients) {
  std::cout << "클라이언트 접속 종료: " << client_fd << std::endl;
  close(client_fd)
}

Server::Server(int port_num, char** port) {}

Server::~Server() {}

// public method;

void Server::myBind() {}
void Server::myListen() {}

int Server::mySetSocketOpt(int val, int fd) {}

bool Server::IsServer(int fd) {}

void Server::turnOn() {}

void Server::showYourSelf() {}

void Server::fileSetting(const char* path) {}
