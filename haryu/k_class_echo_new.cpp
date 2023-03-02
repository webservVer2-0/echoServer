#include "k_class_page.hpp"

// 목표 1 : 페이지 전달 서버 구축하기
// 목표 2 : 다중 포트 상태로 값이 들어오면 서버 클래스가 자동으로 멀티 포트로
// 운영됨 목표 3 : 접근하면 index.html 전달하기 목표 4 : index_comp.html
// 전닳시키고 커낵션 종료 시켜보기

void Server::exitWithPerror(const String& msg_) {
  CER << msg_ << CEND;
  exit(EXIT_FAILURE);
}
void Server::changeEvents(VEC<struct kevent>& changeList_, uintptr_t ident,
                          int16_t filter, uint16_t flags, uint16_t fflags,
                          intptr_t data, void* udata) {
  struct kevent tempEvent;

  EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
  changeList_.push_back(tempEvent);
}
void Server::disconnectClients(int clientFd, MAP<int, String>& clients_) {
  std::cout << "client disconnected : " << clientFd << std::endl;
  close(clientFd);
  clients_.erase(clientFd);
}
Server::Server(int portNumber_, char** port_) : msg_("") {
  if (portNumber_ == 1) {
    COUT << "./k_http_server {port 1} {port 2} ... " << CEND;
    exit(1);
  }
  serverSocket_ = new int[portNumber_];
  if (serverSocket_ == nullptr) {
    CER << "constructor error" << CEND;
    exit(1);
  }
  serverAddr_ = new sockaddr_in[portNumber_];
  if (serverAddr_ == nullptr) {
    CER << "constructor error" << CEND;
    exit(1);
  }
  for (int i = 0; i + 1 < portNumber_; i++) {
    serverSocket_[i] = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket_[i] == -1) {
      exitWithPerror("socket() error\n" + String(strerror(errno)));
    }

    memset(&serverAddr_[i], 0, sizeof(serverAddr_[i]));
    serverAddr_[i].sin_family = AF_INET;
    serverAddr_[i].sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr_[i].sin_port = htons(atoi(port_[i + 1]));
  }
  portnum_ = portNumber_ - 1;
}

Server::~Server() {
  delete[] serverSocket_;
  delete[] serverAddr_;
}
void Server::myBind() {
  int ret;

  for (int i = 0; i < portnum_; i++) {
    ret = bind(serverSocket_[i],
               reinterpret_cast<struct sockaddr*>(&serverAddr_[i]),
               sizeof(serverAddr_[i]));
    std::cout << serverSocket_[i] << " : " << sizeof(serverAddr_[i])
              << std::endl;
    if (ret == -1) {
      exitWithPerror("bind() error\n" + String(strerror(errno)));
    }
  }
  return;
}

void Server::myListen() {
  int ret;

  for (int i = 0; i < portnum_; i++) {
    ret = listen(serverSocket_[i], 10);
    if (ret == -1) {
      exitWithPerror("listen() error\n" + String(strerror(errno)));
    }
    fcntl(serverSocket_[i], F_SETFL, O_NONBLOCK);
  }
}

int Server::mySetSockopt(int val, int fd) {
  int ret;

  if (val == SO_REUSEADDR) {
    int bf;

    for (int i = 0; i < portnum_; i++) {
      ret = setsockopt(serverSocket_[i], SOL_SOCKET, SO_REUSEADDR, &bf,
                       sizeof(bf));
      if (ret == -1) {
        exitWithPerror("setsockopt(SO_REUSERADDR) error\n" +
                       String(strerror(errno)));
      }
      COUT << "Socket(" << serverSocket_[i]
           << ")setting is succesed : SO_REUSEADDR" << CEND;
    }
    return (0);
  } else if (val == SO_LINGER) {
    struct linger loption_;

    loption_.l_onoff = 1;
    loption_.l_linger = 0;

    ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &loption_, sizeof(loption_));
    if (ret == -1) {
      exitWithPerror("setsockopt(SO_LINGER) error\n" + String(strerror(errno)));
    }
    COUT << "Socket setting is succesed : SO_LINGER(" << fd << ")" << CEND;
    return (0);
  } else {
    CER << "Socket setting is failed." << CEND;
    return (-1);
  }
}

bool Server::IsServer(int fd) {
  int ret = 0;

  for (int i = 0; i < portnum_; i++) {
    if (serverSocket_[i] == fd) ret += 1;
  }
  return (ret);
}

void Server::turnOn() {
  kq_ = kqueue();
  if (kq_ == -1) {
    exitWithPerror("kq_ueue() error\n" + String(strerror(errno)));
  }

  for (int i = 0; i < portnum_; i++) {
    changeEvents(changeList_, serverSocket_[i], EVFILT_READ, EV_ADD | EV_ENABLE,
                 0, 0, NULL);
    COUT << "K-Page Server Started : " << serverSocket_[i] << CEND;
  }

  int newEvents_ = 0;
  struct kevent* currEvent_;
  while (true) {
    std::cout << "new event : " << newEvents_ << std::endl;
    newEvents_ = kevent(kq_, &changeList_[0], changeList_.size(), eventList_,
                        EVENT_MAX, nullptr);
    if (newEvents_ == -1) {
      exitWithPerror("kevent() error\n" + String(strerror(errno)));
    }

    changeList_.clear();

    for (int i = 0; i < newEvents_; ++i) {
      currEvent_ = &eventList_[i];
      if (currEvent_->flags & EV_ERROR) {
        for (int i = 0; i < portnum_; i++) {
          if (currEvent_->ident == serverSocket_[i])
            exitWithPerror("server socket error");
          else {
            CER << "clients_ socket error" << CEND;
            disconnectClients(currEvent_->ident, clients_);
          }
        }
      } else if (currEvent_->filter == EVFILT_READ) {
        if (IsServer(currEvent_->ident)) {
          /* Accept new client */
          int clientS_ocket(accept(currEvent_->ident, NULL, NULL));

          if (clientS_ocket == -1) {
            exitWithPerror("accept() error\n" + String(strerror(errno)));
          }
          COUT << "accept new client : " << clientS_ocket << CEND;
          fcntl(clientS_ocket, F_SETFL, O_NONBLOCK);

          /* add event for client socket - add read && write event */
          changeEvents(changeList_, clientS_ocket, EVFILT_READ,
                       EV_ADD | EV_ENABLE | EV_EOF, 0, 0, NULL);
          mySetSockopt(SO_LINGER, clientS_ocket);
          clients_[clientS_ocket] = "";
        } else if (clients_.find(currEvent_->ident) != clients_.end()) {
          /* read data from clients_ */
          char* buf = new char[currEvent_->data];
          int n = recv(currEvent_->ident, buf, currEvent_->data, 0);
          if (n == 0) {
            // disconnectClients(currEvent_->ident, clients_);
            clients_[currEvent_->ident].clear();
          } else {
            // buf[n] = '\0';
            // clients_[currEvent_->ident].append(buf);
            // COUT << "received data from " << currEvent_->ident << ":\n"
            //      << clients_[currEvent_->ident] << CEND;
            // size_t firstLine_ = clients_[currEvent_->ident].find('\n');
            // String firstLineString_ =
            //     clients_[currEvent_->ident].substr(0, firstLine_);
            // size_t srcStart = firstLineString_.find(' ');
            // size_t srcEnd = firstLineString_.find(' ', srcStart + 1);
            // String srcLine = firstLineString_.substr(srcStart + 1, srcEnd -
            // 4);
            /* send Data to client*/
            // if (srcLine == "/") {
            // std::map<int, String>::iterator it =
            //     clients_.find(currEvent_->ident);
            // if (it != clients_.end()) {
            //   int n;
            n = send(currEvent_->ident, buf, currEvent_->data, 0);
            if (n == -1) {
              disconnectClients(currEvent_->ident, clients_);
              exitWithPerror("client write error");
            } else if (n == 0)
              disconnectClients(currEvent_->ident, clients_);
          }
          //   else clients_[currEvent_->ident].clear();
          // }
        }
      }
    }
  }
}

void Server::showYourSelf() {
  for (int i = 0; i < portnum_; i++) {
    COUT << "Server Socket : " << this->serverSocket_[i] << CEND;
    COUT << "Server Port : " << ntohs(this->serverAddr_[i].sin_port) << CEND;
  }
}

void Server::fileSetting(const char* path) {
  std::ifstream httpmsg_;
  std::ifstream entity;
  String entityloc(DEFAULTLLOCATION);

  httpmsg_.open(String(DEFAULTLLOCATION).append(HTTP), std::ifstream::in);
  entityloc.append(path);

  COUT << entityloc << CEND;

  entity.open(entityloc.c_str(), std::ifstream::in);

  int lengthFirst;
  int lengthSecond;

  httpmsg_.seekg(0, httpmsg_.end);
  lengthFirst = httpmsg_.tellg();
  httpmsg_.seekg(0, httpmsg_.beg);

  entity.seekg(0, entity.end);
  lengthSecond = entity.tellg();
  entity.seekg(0, entity.beg);

  char* buffer1 = new char[lengthFirst + 1];
  char* buffer2 = new char[lengthSecond + 1];
  buffer1[lengthFirst] = '\0';
  buffer2[lengthSecond] = '\0';

  httpmsg_.read(buffer1, lengthFirst);
  entity.read(buffer2, lengthSecond);

  msg_.append(buffer1);
  msg_.push_back(static_cast<char>(13));
  msg_.append("\n");
  msg_.append(buffer2);
  delete[] buffer1;
  delete[] buffer2;
}

int main(int ac, char** av) {
  Server myServer(ac, av);

  myServer.myBind();
  myServer.myListen();
  myServer.showYourSelf();
  myServer.mySetSockopt(SO_REUSEADDR, 0);
  myServer.fileSetting(INDEX);
  myServer.turnOn();

  return 0;
}