#ifndef k_class_page_hpp_
#define k_class_page_hpp_

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>  // atoi
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

#define String std::string
#define COUT std::cout
#define CEND std::endl
#define CER std::cerr
#define VEC std::vector
#define MAP std::map

#ifndef EVENT_MAX
#define EVENT_MAX 100
#endif

#ifndef REUSE
#define REUSE 1
#endif

#ifndef LINGER
#define LINGER 1
#endif

#ifndef DEFAULTLOCATION
#define DEFAULTLLOCATION \
  "/Users/haryu/workspace/echoServer/haryu/study/kserver/"
#endif

#define HTTP "httpmsg"
#define INDEX "index.html"
#define INDEXC "index_comp.html"
#define SRC "/src/"

class Server {
 private:
  int* serverSocket_;
  struct sockaddr_in* serverAddr_;
  int portnum_;
  int kq_;
  MAP<int, String> clients_;
  VEC<struct kevent> changeList_;
  struct kevent eventList_[EVENT_MAX];
  String msg_;

  void exitWithPerror(const String& msg_);
  void changeEvents(VEC<struct kevent>& changeList_, uintptr_t ident,
                    int16_t filter, uint16_t flags, uint16_t fflags,
                    intptr_t data, void* udata);
  void disconnectClients(int clientFd, MAP<int, String>& clients_);

 public:
  Server(int portNumber_, char** port_);
  ~Server(void);
  void myBind();
  void myListen();
  int mySetSockopt(int val, int fd);
  bool IsServer(int fd);
  void turnOn();
  void showYourSelf();
  void fileSetting(const char* path);
};
#endif