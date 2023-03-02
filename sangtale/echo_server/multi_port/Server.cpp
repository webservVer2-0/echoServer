#include "Server.hpp"
#include "syscall.hpp"

Server::Server(int port_number, char **ports) : _message("") {
  if (port_number == 1)
    exit_with_error("Usage\n $> ./multi_port_serv {port N}");
  _serverSocket = new int[port_number];
  if (_serverSocket == nullptr)
    exit_with_error("memory allocator error(_serverSocket)");
  _serverAddr = new sockaddr_in[port_number];
  if (_serverAddr == nullptr)
    exit_with_error("memory allocator error(_serverAddr)");
  for (int i = 0; i < port_number - 1; ++i) {
    _serverSocket[i] = __socket_init();
    const int reuse_add_optval = 1; /* SO_REUSEADDR 옵션에 해당하는 optval 값*/
    __setsockopt_handling(_serverSocket[i], SO_REUSEADDR, &reuse_add_optval);
    __init_server_addr(_serverAddr[i], atoi(ports[i + 1]));
  }
  _portCount = port_number - 1;
}

Server::~Server() {
  delete[] _serverSocket;
  delete[] _serverAddr;
}

inline int is_server_sd(int sd, int port_count, int *server_socket);

/*
  [public]
*/
void Server::event_controller() {
  _kq = __kqueue_handling();
  for (int i = 0; i < _portCount; ++i)
    changeEvents(_changeList, _serverSocket[i], EVFILT_READ, EV_ADD | EV_ENABLE,
                 0, 0, NULL);
  _kevent_controller();
}

void Server::bind_socket() {
  for (int i = 0; i < _portCount; ++i)
    __bind_handling(_serverSocket[i], _serverAddr[i]);
}

void Server::listen_and_fcntl() {
  for (int i = 0; i < _portCount; ++i) {
    __listen_handling(_serverSocket[i]);
    __fcntl_handling(_serverSocket[i]);
  }
}

void Server::print_server_info() {
  std::cout << _portCount << "\n";
  for (int i = 0; i < _portCount; ++i) {
    std::cout << "Server Socket : " << _serverSocket[i] << "\n";
    std::cout << "Server Port : " << ntohs(_serverAddr[i].sin_port) << "\n";
  }
}

/*
  [private]
*/

void Server::_kevent_controller() {
  int new_events = 0;
  struct kevent *curr_event;

  while (true) {
    new_events = _kevent_handling();
    _changeList.clear();
    for (int i = 0; i < new_events; ++i) {
      curr_event = &_eventList[i];
      if (curr_event->flags & EV_ERROR)
        _kevent_error_case_handler(curr_event);
      else if (curr_event->filter == EVFILT_READ) {
        if (is_server_sd(curr_event->ident, _portCount, _serverSocket))
          _accept_and_set_client(curr_event->ident);
        else
          _recv_client(curr_event);
      }
    }
  }
}

/**
 * @brief             curr_event id를 server_socket과 대조해서
 *                    못 찾았으면 클라이언트의 에러
 * @param curr_event
 */
void Server::_kevent_error_case_handler(struct kevent *curr_event) {
  for (int i = 0; i < _portCount; ++i) {
    if (curr_event->ident == _serverSocket[i]) {
      std::string err_msg("server socket error : ");
      err_msg.push_back(_serverSocket[i] + '0');
      exit_with_error(err_msg);
    }
  }
  std::cerr << "clients socket error\n";
  disconnectClients(curr_event->ident, _clients);
}

void Server::_accept_and_set_client(int sd) {
  const int client_socket = __accept_handling(sd);
  std::cout << "accept new client : " << sd << "\n";
  __fcntl_handling(client_socket);
  changeEvents(_changeList, client_socket, EVFILT_READ,
               EV_ADD | EV_ENABLE | EV_EOF, 0, 0, NULL);

  struct linger loption;
  loption.l_onoff = 1;
  loption.l_linger = 0;
  __setsockopt_handling(client_socket, SO_LINGER, &loption);
  _clients[client_socket] = "";
}

void Server::_recv_client(struct kevent *curr_event) {
  char buf[1024];
  int rb = recv(curr_event->ident, buf, sizeof(buf), 0);
  if (rb <= 0)
    _clients[curr_event->ident].clear();
  else {
    buf[rb] = '\0';
    _clients[curr_event->ident].append(buf);
    std::cout << "received data from " << curr_event->ident << ": "
              << _clients[curr_event->ident];
    int sb = send(curr_event->ident, buf, curr_event->data, 0);
    if (sb == -1) {
      disconnectClients(curr_event->ident, _clients);
      std::cerr << "sever send() error\n";
    } else if (sb == 0)
      disconnectClients(curr_event->ident, _clients);
    _clients[curr_event->ident].clear();
  }
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

int Server::_kevent_handling() {
  const int event_max = 8;
  const int new_events = kevent(_kq, &_changeList[0], _changeList.size(),
                                _eventList, event_max, nullptr);
  if (new_events == -1)
    exit_with_error("kevent() error");
  return (new_events);
}

inline int is_server_sd(int sd, int port_count, int *server_socket) {
  for (int i = 0; i < port_count; ++i)
    if (server_socket[i] == sd)
      return (1);
  return (0);
}

//#define DEFAULT_LOCATION                                                       \
//  "/Users/kalabi/webserv/exer/echoServer/sangtale/echo_server/multi_port/"
// #define HTTP "http_msg"

// void Server::file_setting() {
//   std::ifstream http_msg;
//   std::ifstream entity;
//   std::string entity_location(DEFAULT_LOCATION);

//  http_msg.open(std::string(DEFAULT_LOCATION).append(HTTP),
//  std::ifstream::in); entity_location.append(_home);

//  std::cout << entity_location << "\n";
//  entity.open(entity_location.c_str(), std::ifstream::in);

//  int lengthFirst;
//  int lengthSecond;

//  http_msg.seekg(0, http_msg.end);
//  lengthFirst = http_msg.tellg();
//  http_msg.seekg(0, http_msg.beg);

//  entity.seekg(0, entity.end);
//  lengthSecond = entity.tellg();
//  entity.seekg(0, entity.beg);

//  char *buffer1 = new char[lengthFirst + 1];
//  char *buffer2 = new char[lengthSecond + 1];
//  buffer1[lengthFirst] = '\0';
//  buffer2[lengthSecond] = '\0';

//  http_msg.read(buffer1, lengthFirst);
//  entity.read(buffer2, lengthSecond);

//  _message.append(buffer1);
//  _message.push_back(static_cast<char>(13));
//  _message.append("\n");
//  _message.append(buffer2);
//  delete[] buffer1;
//  delete[] buffer2;
//}
