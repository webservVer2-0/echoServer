#ifndef ECHO_SERVER_MULTI_HPP
#define ECHO_SERVER_MULTI_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifndef EVENT_MAX
#define EVENT_MAX 100
#endif

class Server {
 private:
  int* server_socket_;
  struct sockaddr_in* server_addr_;
  int port_num_;
  int kq_;
  std::map<int, std::string> clients_;
  std::vector<struct kevent> change_list_;
  struct kevent event_list_[EVENT_MAX];
  std::string msg_;

  void exitWithPerror(const std::string& msg);
  void changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident,
                    int16_t filter, uint16_t flags, uint16_t fflags,
                    intptr_t data, void* udata);
  void disconnectClients(int client_fd, std::map<int, std::string>& clients);

 public:
  Server(int port_num, char** port);
  ~Server();
  void myBind();
  void myListen();
  int mySetSocketOpt(int val, int fd);
  bool IsServer(int fd);
  void turnOn();
  void showYourSelf();
  void fileSetting(const char* path);
};

#endif
