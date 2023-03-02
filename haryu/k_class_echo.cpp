#include <sys/types.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h> // atoi
#include <unistd.h>

#include <ctime>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>

#define String std::string
#define COUT std::cout 
#define CEND std::endl
#define CER std::cerr
#define VEC std::vector
#define MAP std::map

#ifndef EVENT_MAX
# define EVENT_MAX 8
#endif

#ifndef REUSE
# define REUSE 1
#endif 

#ifndef LINGER
# define LINGER 1
#endif

#ifndef DEFAULTLOCATION
# define DEFAULTLLOCATION "/Users/haryu/workspace/echoServer/haryu/study/kserver/"
#endif 

#define HTTP "httpmsg"
#define INDEX "index.html"
#define INDEXC "index_comp.html"
#define SRC "/src/"

// 목표 1 : 페이지 전달 서버 구축하기
// 목표 2 : 다중 포트 상태로 값이 들어오면 서버 클래스가 자동으로 멀티 포트로 운영됨 
// 목표 3 : 접근하면 index.html 전달하기 
// 목표 4 : index_comp.html 전달시키고 커낵션 종료 시켜보기

class Server {
private : 
	int serverSocket;
	struct sockaddr_in serverAddr;
	int kq;
	MAP<int, String> clients;
	VEC<struct kevent> changeList;
	struct kevent eventList[EVENT_MAX];
	String msg;

	int setSocketOptions();
	void exitWithPerror(const String& msg) {
		CER << msg << CEND;
		exit(EXIT_FAILURE);
	}
	void changeEvents(VEC<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint16_t fflags, intptr_t data, void* udata) {
		struct kevent tempEvent;

		EV_SET(&tempEvent, ident, filter,  flags, fflags, data, udata);
		changeList.push_back(tempEvent);
	}
	void disconnectClients(int clientFd, MAP<int, String>& clients) {
		std::cout << "client disconnected : " << clientFd << std::endl;
		close(clientFd);
		clients.erase(clientFd);
	}
	// void readEvent();
	// void writeEvent(String msg);
public :
	Server(int portNumber, char **port) : msg("") {
		if (portNumber != 2) { 
			COUT << "next time baby" << CEND;
			exit (1);
		}
		serverSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1) { exitWithPerror("socket() error\n" + String(strerror(errno))); }

		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(atoi(port[1]));
	}
	void	myBind() {
		int ret;

		ret = bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr));
		if (ret == -1) { exitWithPerror("bind() error\n" + String(strerror(errno))); }
		return ;
	}
	void	myListen() {
		int ret;
		
		ret = listen(serverSocket, 5);
		if (ret == -1) { exitWithPerror("listen() error\n" +  String(strerror(errno))); }
		return ;
		fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	}
	int		setSockopt(int val) {
			int ret;
		if (val == SO_REUSEADDR) {
			int bf;

			ret = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &bf, sizeof(bf));
			if (ret == -1) { exitWithPerror("setsockopt(SO_REUSERADDR) error\n" + String(strerror(errno))); }
			COUT << "Socket setting is succesed : SO_REUSEADDR" << CEND;
			return (0);
		}
		else if (val == SO_LINGER) {
			struct linger loption;

			loption.l_onoff = 1;
			loption.l_linger = 5;

			ret = setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, &loption, sizeof(loption));
			if (ret == -1) { exitWithPerror("setsockopt(SO_LINGER) error\n" + String(strerror(errno))); }
			COUT << "Socket setting is succesed : SO_LINGER" << CEND;
			return (0);
		}
		else {
			CER << "Socket setting is failed." << CEND;
			return (-1);
		}
		
	}
	void	turnOn() {
		kq = kqueue();
		if (kq == -1) { exitWithPerror("kqueue() error\n" + String	(strerror(errno))); }

		changeEvents(changeList, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		COUT << "K_Page Server Started" << CEND;

		int newEvents;
		struct kevent * currEvent;
		while(true) {
			newEvents = kevent(kq, &changeList[0], changeList.size(), eventList, EVENT_MAX, NULL);
			if (newEvents == -1) { exitWithPerror("kevent() error\n" + String(strerror(errno))); }

			changeList.clear();

			for (int i = 0; i < newEvents; ++i) {
				currEvent = &eventList[i];
				if (currEvent->flags & EV_ERROR) {
					if (currEvent->ident == serverSocket)
						exitWithPerror("server socket error");
					else {
						CER << "clients socket error" << CEND;
						disconnectClients(currEvent->ident, clients);
					}
				}
				else if (currEvent->filter == EVFILT_READ) {
					if (currEvent->ident == serverSocket) {
						/* Accept new client */
						int clientSocket(accept(serverSocket, NULL, NULL));
						if (clientSocket == -1) { exitWithPerror("accept() error\n" + String(strerror(errno))); }
						COUT << "accept new client : " << clientSocket << CEND;
						fcntl(clientSocket, F_SETFL, O_NONBLOCK);

						/* add event for client socket - add read && write event */
						changeEvents(changeList, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						changeEvents(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						clients[clientSocket] = "";
					}
					else if (clients.find(currEvent->ident) != clients.end()) {
						/* read data from clients */
						char buf[1024];
						int n = recv(currEvent->ident, buf, sizeof(buf), 0);
						if (n <= 0) {
							if (n < 0) { CER << "client read error!" << CEND; }
							disconnectClients(currEvent->ident, clients);
						}
						else {
							buf[n] = '\0';
							clients[currEvent->ident].append(buf);
							COUT << "received data from " << currEvent->ident << ":\n" << clients[currEvent->ident] << CEND;
						}
					}
				}
				else if (currEvent->filter == EVFILT_WRITE) {
					/* send Data to client*/
					std::map<int, String>::iterator it = clients.find(currEvent->ident);
					if (it != clients.end()) {
						int n;
						std::cout << strlen(msg.c_str()) << std::endl;
						n = send(currEvent->ident, msg.c_str(), strlen(msg.c_str()), MSG_DONTWAIT);
						if (n == -1) {
							CER << "send() for clients error : " << currEvent->ident << " : " << clients[currEvent->ident] << CEND;
							disconnectClients(currEvent->ident, clients);
						}
						disconnectClients(currEvent->ident, clients);
						clients[currEvent->ident].clear();
					}
				}
			}
		}
	}
	void	showYourSelf() {
		COUT << "Server Socket : " << this->serverSocket << CEND;
		COUT << "Server Address : " << ntohl(this->serverAddr.sin_addr.s_addr) << CEND;
		COUT << "Server Port : " << ntohs(this->serverAddr.sin_port) << CEND;
	}

	void	fileSetting(const char * path) {
		std::ifstream httpmsg;
		std::ifstream entity;
		String entityloc(DEFAULTLLOCATION);

		httpmsg.open(String(DEFAULTLLOCATION).append(HTTP), std::ifstream::in);
		entityloc.append(path);

		COUT << entityloc << CEND;

		entity.open(entityloc.c_str(), std::ifstream::in);

		int lengthFirst;
		int lengthSecond;

		httpmsg.seekg(0, httpmsg.end);
		lengthFirst = httpmsg.tellg();
		httpmsg.seekg(0, httpmsg.beg);

		entity.seekg(0, entity.end);
		lengthSecond = entity.tellg();
		entity.seekg(0, entity.beg);

		char * buffer1 = new char[lengthFirst + 1];
		char * buffer2 = new char[lengthSecond + 1];
		buffer1[lengthFirst] = '\0';
		buffer2[lengthSecond] = '\0';

		httpmsg.read(buffer1, lengthFirst);
		entity.read(buffer2, lengthSecond);

		msg.append(buffer1);
		msg.push_back(static_cast<char>(13));
		msg.append("\n");
		msg.append(buffer2);
	}
};

int main(int ac, char **av) {
	Server myServer(ac, av);
	
	myServer.myBind();
	myServer.myListen();
	myServer.showYourSelf();
	myServer.setSockopt(SO_REUSEADDR);
	myServer.setSockopt(SO_LINGER);
	myServer.fileSetting(INDEX);
	myServer.turnOn();
	
	return 0;
}