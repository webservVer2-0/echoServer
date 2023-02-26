# 신~~~~~~~~~~~~~~~~~~~~~~~~~~나는 에코 서버

1. Makefile <br>
2. echoServer를 실행
```bash
./echoServer
```
3. client를 실행
```bash
./client 127.0.0.1 
```
4. client에 메시지 입력
```bash
./client 127.0.0.1 
$> hello world
```
5. server를 킨 터미널에서 수신 확인




# chatGPT와 묻고 답한 것들

## 커널(Kernel)과 커널의 역할
![](https://velog.velcdn.com/images/yisemo/post/b2235834-06e9-4628-b229-0f33d20a84ee/image.jpeg)

커널(Kernel)은 운영체제에서 가장 핵심적인 부분으로, 하드웨어와 응용 프로그램 간의 상호작용을 관리합니다.

커널은 시스템 자원을 효율적으로 관리하며, 응용 프로그램이 하드웨어를 사용할 수 있도록 인터페이스를 제공합니다. 또한, 커널은 시스템 보안과 안정성을 유지하고, 프로세스 간의 통신을 관리하며, 입출력 작업을 처리합니다.

커널은 일반적으로 운영체제의 핵심 모듈 중 하나로, 다양한 운영체제에서 사용됩니다. 우리가 흔히 사용하는 운영체제인 Windows, macOS, Linux 등에서도 모두 커널이 존재합니다.

<br>

## 커널 이벤트 큐(Kernel Event Queue)

커널 이벤트 큐(Kernel Event Queue)는 운영체제 커널 내부에서 발생하는 이벤트들을 저장하고 관리하는 자료구조입니다.

커널 이벤트 큐는 운영체제 내부에서 발생하는 다양한 이벤트들을 저장합니다. 예를 들어, 프로세스나 스레드의 생성, 종료, 입출력 작업, 인터럽트 등의 이벤트들이 커널 이벤트 큐에 저장됩니다.

커널 이벤트 큐는 운영체제의 다른 부분에서 이벤트를 처리하기 전에 먼저 저장해 놓아야 하는 경우가 많습니다. 예를 들어, 인터럽트가 발생하면 커널은 인터럽트 이벤트를 커널 이벤트 큐에 저장하고, 이후 인터럽트 처리기가 이벤트를 처리할 때까지 대기합니다.

커널 이벤트 큐는 다양한 운영체제에서 사용되며, 이벤트 처리를 위해 다양한 알고리즘과 자료구조를 활용합니다. 이를 통해 운영체제의 안정성과 성능을 유지하며, 다양한 이벤트를 효율적으로 처리합니다.

<br>


# 필요한 시스템 콜, 자료

## socket()

```cpp
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```
### 요약
socket()은 네트워크 프로그래밍에서 소켓(Socket)을 생성하는 시스템 콜(System Call)입니다.


소켓은 네트워크 상에서 프로세스 간 통신을 가능하게 해주는 역할을 합니다. 이때, 소켓을 생성하기 위해서는 socket() 시스템 콜을 호출해야 합니다.

socket() 시스템 콜은 아래와 같은 인자를 받습니다.

**domain**
소켓의 프로토콜 체계(Protocol Family)를 지정합니다. 예를 들어, IPv4를 사용하는 경우 `AF_INET` 혹은 `PF_INET`으로 지정합니다.

> **AF_INET과 PF_INET의 차이?**
결론 : 차이 없음 [[참고](https://www.bangseongbeom.com/af-inet-vs-pf-inet.html)]
![](https://velog.velcdn.com/images/yisemo/post/49d860bf-b437-44b0-8e70-60618a5e9235/image.png)


**type**
소켓의 타입을 지정합니다. 예를 들어, TCP 소켓을 사용하는 경우 `SOCK_STREAM`으로 지정합니다.
**protocol**
소켓에서 사용할 프로토콜을 지정합니다. 일반적으로 `0`으로 지정하여 시스템이 알아서 선택하도록 합니다.


### **return value**
socket() 시스템 콜은 성공하면 소켓 파일 디스크립터(Socket File Descriptor)를 반환합니다. 이 파일 디스크립터를 이용하여, 프로세스는 소켓을 이용한 네트워크 통신을 할 수 있습니다.


<br>

## sockaddr_in

```cpp
struct sockaddr_in {
    short sin_family;           // 주소 체계 (Address Family)
    unsigned short sin_port;    // 포트 번호
    struct in_addr sin_addr;    // IP 주소
    char sin_zero[8];           // 구조체 크기를 맞추기 위한 패딩
};


/**
	@ 초기화의 예시
     1. family에 소켓 주소를 저장
     2. IP 주소를 INADDR_ANY(0.0.0.0 즉, 루프백 IP)로 초기화
     3. 8080 포트를 열고
*/
void __init_server_addr(struct sockaddr_in *server_addr) {
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr->sin_family = AF_INET;
  server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr->sin_port = htons(8080);
}

```

sockaddr_in 구조체는 네트워크 프로그래밍에서 사용되는 IPv4 소켓 주소를 표현하는 구조체입니다. 아래는 sockaddr_in 구조체의 멤버들과 간단한 설명입니다.

- **sin_family** : 소켓 주소 체계를 나타내는 멤버입니다. IPv4 주소 체계를 사용하는 경우 `AF_INET`으로 설정됩니다.
- **sin_port** : 포트 번호를 나타내는 멤버입니다. 포트 번호는 16비트의 부호 없는 정수(unsigned short)로 표현됩니다. `htons()` 함수를 사용하여 호스트 바이트 오더에서 네트워크 바이트 오더로 변환해야 합니다.
> **바이트 오더(Byte order)**
http://www.tcpschool.com/c/c_refer_endian
- **sin_addr** : IPv4 주소를 나타내는 `in_addr` 구조체의 멤버입니다. 이 멤버는 `in_addr` 구조체의 `in_addr_t` 타입으로 표현된 IPv4 주소값을 갖습니다.
- **sin_zero** : 구조체 크기를 맞추기 위한 패딩입니다. 이 멤버는 주소 체계(sin_family)와 포트 번호(sin_port) 이후, IPv4 주소(sin_addr)와 구분하기 위해 추가된 멤버입니다. 값을 설정할 필요가 없으며, 일반적으로 0으로 초기화합니다.

### htonl(), htons()
htonl()은 32비트 데이터를 네트워크 바이트 오더(Network Byte Order)로, htons() 함수는 16비트 데이터를 네트워크 바이트 오더로 변환하는 함수입니다. (*정확히는 매크로입니다.*)


네트워크 바이트 오더는 다른 종류의 컴퓨터와 통신할 때, 데이터의 바이트 오더를 통일시키기 위한 국제적인 표준입니다. 네트워크 바이트 오더에서는 바이트가 높은 자리수에서 낮은 자리수로 표시됩니다. 예를 들어, `0x12345678` 값은 네트워크 바이트 오더에서는 `0x78 0x56 0x34 0x12`와 같이 표시됩니다.

htonl() 함수는 호스트 바이트 오더(Host Byte Order)에서 네트워크 바이트 오더로 변환하는 역할을 합니다. 호스트 바이트 오더는 컴퓨터의 CPU 아키텍처나 운영체제 등에 따라 다르게 결정됩니다. 예를 들어, x86 아키텍처의 CPU는 리틀 엔디안(Little Endian) 방식을 사용하므로, 0x12345678 값은 x86에서는 0x78 0x56 0x34 0x12와 같이 저장됩니다.

따라서, htonl(), htons() 함수는 호스트 바이트 오더에서 네트워크 바이트 오더로 변환하기 위해, 현재 시스템의 바이트 오더를 확인하여 필요에 따라 바이트 오더를 바꾸는 작업을 수행합니다. htonl() 함수는 32비트 값(unsigned long)을 인자로 받고, htons()는 16비트 값을 인자로 받습니다. 이후 네트워크 바이트 오더로 변환된 값을 반환합니다.

<br>


## bind()

```cpp
#include <sys/socket.h>

int bind(int socket,
	const struct sockaddr *address, socklen_t address_len);
```

*bind() 함수는 소켓에 IP 주소와 포트 번호를 할당하는 역할을 합니다.*

소켓은 인터넷을 통해 데이터를 주고받기 위한 통로로, 소켓에는 IP 주소와 포트 번호가 필요합니다. IP 주소는 데이터를 전송하는 대상 컴퓨터의 주소를 나타내고, 포트 번호는 대상 컴퓨터의 어느 프로세스에 데이터를 전송할지를 나타냅니다.

bind() 함수는 소켓을 생성한 후에 호출됩니다. `bind()` 함수는 인자로 소켓 디스크립터, `sockaddr` 구조체 포인터, 구조체의 크기를 받습니다. `sockaddr` 구조체는 IP 주소와 포트 번호를 담고 있는 구조체로, 일반적으로 `sockaddr_in` 구조체를 사용합니다.

`bind()` 함수는 소켓 디스크립터에 IP 주소와 포트 번호를 할당하는 역할을 합니다. IP 주소는 `sockaddr_in` 구조체의 `sin_addr` 멤버에 할당되고, 포트 번호는 `sin_port` 멤버에 할당됩니다. 이후에 해당 소켓을 사용하여 데이터를 전송할 때는, 해당 IP 주소와 포트 번호를 가진 대상 컴퓨터의 소켓과 데이터를 주고받게 됩니다.

**return value**
`bind()` 함수는 성공하면 `0`을 반환하고, 실패하면 `-1`을 반환합니다. 또한 실패한 경우에는 `errno` 변수에 에러 코드가 설정됩니다.

<br>

## listen()
```cpp
#include <sys/socket.h>
int listen(int socket, int backlog);
```

#### 인자
1. 소켓 파일 디스크립터(SD)
2. 대기하고 수락을 기다리는 최대 연결 수

#### return value
성공 시 `0`, 실패 시 `-1`

또한 실패하면 `errno`를 재설정

>
EADDRINUSE: 지정된 주소가 이미 사용 중입니다.
EBADF: 소켓 파일 설명자가 유효한 파일 설명자가 아닙니다.
EINVAL: 소켓이 수신에 유효한 상태가 아닙니다.
ENOTSOCK: 파일 설명자가 소켓을 참조하지 않습니다.

#### 간단한 예제

```cpp
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
   int server_fd, new_socket;
   struct sockaddr_in address;
   int addrlen = sizeof(address);

   // 소켓 생성
   server_fd = socket(AF_INET, SOCK_STREAM, 0);

   // 소켓을 포트에 바인딩
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(8080);
   bind(server_fd, (struct sockaddr *)&address, addrlen);

   // 들어오는 연결을 기다리면서 소켓을 수동 모드로 설정
   if (listen(server_fd, 3) < 0)
   	exit(perror("listen() fail\n"));
   return 0;
}
```
<br>

## fcntl()

```cpp
#include <fcntl.h>

int fcntl(int fildes, int cmd, ...);
```

`fcntl()` 함수(file control의 줄임말)는 파일의 속성을 변경하거나 fd의 동작을 제어하거나 fd를 복제하는 데 사용할 수 있습니다.


### 인자
1. 작동 대상(fd)
2. 수행할 작업을 지정하는 명령
3. 명령에 따라 추가 정보를 제공하는 가변 인자

`F_DUPFD`: 파일 설명자를 복제하여 원래 설명자와 동일한 열린 파일 설명을 참조하는 새 설명자를 생성합니다.
`F_GETFL`: 파일 가져오기 파일 설명자와 관련된 상태 플래그.
`F_SETFL`: 파일 설명자와 관련된 파일 상태 플래그를 설정합니다.
`F_GETFD`: 파일과 관련된 파일 설명자 플래그를 가져옵니다. 설명자.
`F_SETFD`: 파일 설명자와 관련된 파일 설명자 플래그를 설정합니다.

<br>

#### **[간단한 예제]**

```cpp
#include <fcntl.h>
#include <sys/socket.h>

int main() {
   int server_fd, flags;
   
   // Create socket
   server_fd = socket(AF_INET, SOCK_STREAM, 0);

   // Set socket to non-blocking mode
   flags = fcntl(server_fd, F_GETFL, 0);
   fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

   // Do something with the socket...
   return (0);
}
```




# 구현

### 헤더와 시스템 콜 핸들링
```cpp
#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <vector>

void exit_with_error(const std::string &msg);

int __socket_init();
void __init_server_addr(struct sockaddr_in &server_addr);
void __bind_handling(int server_socket, struct sockaddr_in &server_addr);
void __listen_handling(int sd);
void __fcntl_handling(int sd);
int __kqueue_handling();
int __accept_handling(int sd);

inline void exit_with_error(const std::string &msg) {
  std::cerr << msg << "\n";
  exit(EXIT_FAILURE);
}

/**
 * @brief 소켓 디스크립터(SD)를 반환 and 시스템 콜 에러 핸들링
 * @return socket()의 return value, SD(FD)
 */
int __socket_init() {
  const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1)
    exit_with_error("socket() error\n" + std::string(strerror(errno)));
  return (server_socket);
}

/**
 * @brief 서버 어드레스를 초기화
 * @param server_addr
 */
void __init_server_addr(struct sockaddr_in &server_addr) {
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(8080);
}

/**
 * @brief 소켓에 IP 주소와 포트 번호를 할당
 * @param sd, server_addr
 */
void __bind_handling(int server_socket, struct sockaddr_in &server_addr) {
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == -1)
    exit_with_error("bind() error\n" + std::string(strerror(errno)));
}

/**
 * @brief 서버 소켓에서 클라이언트의 접속을 대기
 *        wait queue는 대기 큐로, 크기는 5
 * @param sd
 */
void __listen_handling(int sd) {
  const int count_wait_queue = 5;
  if (listen(sd, count_wait_queue) == -1)
    exit_with_error("listen() error\n" + std::string(strerror(errno)));
}

void __fcntl_handling(int sd) {
  if (fcntl(sd, F_SETFL, O_NONBLOCK) == -1)
    exit_with_error("fcntl() error\n" + std::string(strerror(errno)));
}

int __kqueue_handling() {
  const int kq = kqueue();
  if (kq == -1)
    exit_with_error("kqueue() error\n" + std::string(strerror(errno)));
  return (kq);
}

int __accept_handling(int sd) {
  const int client_socket = accept(sd, NULL, NULL);
  if (client_socket == -1)
    exit_with_error("accept() error\n" + std::string(strerror(errno)));
  return (client_socket);
}

```


### 메인 로직 

```cpp
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
```


<br>

### 클라이언트

```cpp
#include "echo_server.hpp"

static void error_handling(const char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

int main(int ac, char *av[]) {
  int sock;
  struct sockaddr_in serv_addr;
  char message[100];

  if (ac != 3) {
    printf("argument error\n");
    printf("usage : ./client 127.0.0.1 8080");
    exit(EXIT_FAILURE);
  }

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    error_handling("socket() error");
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = PF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(av[1]);
  serv_addr.sin_port = htons(atoi(av[2]));

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    error_handling("connect() error.");
  }

  int n;
  n = scanf("%[^\n]s", message);
  if (n == -1) {
    error_handling("scanf() error");
  }
  n = write(sock, message, sizeof(message));
  if (n == -1) {
    error_handling("write() error");
  }
  close(sock);
  return (0);
}

```


kevent 관련 글은 차후 추가 하겠습니다.


<br>

참고 : [hyeonski님 tistory](https://hyeonski.tistory.com/9)