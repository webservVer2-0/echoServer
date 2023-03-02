#pragma once

#include "syscall.hpp"
#include <cstdlib>
#include <fstream>
#include <map>
#include <vector>

#define EVENT_MAX 100

class Server {
private:
  int _portCount;
  int _kq;
  std::map<int, std::string> _clients;
  std::vector<struct kevent> _changeList;
  struct kevent _eventList[EVENT_MAX];
  std::string _message;
  std::string _home;

  /* memory alloc */
  int *_serverSocket;
  struct sockaddr_in *_serverAddr;

  /* event_controller() */
  int _kevent_handling();
  void _accept_and_set_client(int sd);
  void _kevent_controller();
  void _kevent_error_case_handler(struct kevent *curr_event);
  void _recv_client(struct kevent *curr_event);
  void changeEvents(std::vector<struct kevent> &changeList_, uintptr_t ident,
                    int16_t filter, uint16_t flags, uint16_t fflags,
                    intptr_t data, void *udata);
  void disconnectClients(int clientFd, std::map<int, std::string> &clients_);

public:
  Server(int port_number, char **ports);
  ~Server();

  /*
       utils
  */
  void bind_socket();
  void listen_and_fcntl();
  void print_server_info();
  void event_controller();
};