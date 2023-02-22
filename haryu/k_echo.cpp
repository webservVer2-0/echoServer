/* Copyright 2023-01-09 haryu */
#include <sys/types.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <ctime>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#define String std::string

void exitWithPerror(const String& msg) {
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

void changeEvents(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint16_t fflags, intptr_t data, void *udata){
	struct kevent tempEvent;

	EV_SET(&tempEvent, ident, filter,  flags, fflags, data, udata);
	changeList.push_back(tempEvent);
}

void disconnectClient(int clientFd, std::map<int, String> & clients) {
	std::cout << "client disconnected : " << clientFd << std::endl;
	close(clientFd);
	clients.erase(clientFd);
}

int main(int ac, char **av) {
    int serverSocket;
	int bf;
	int rn;
	struct sockaddr_in serverAddr;

	/* server 소켓 초기화, listen 상태 */
	if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		exitWithPerror("socket() error\n" + String(strerror(errno)));

	/**
	 * @brief bind error 해결용
	 * 
	 */
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &bf, (socklen_t)rn);


	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(4242);

	if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1)
		exitWithPerror("bind() error\n" + String(strerror(errno)));
	if (listen(serverSocket, 5) == -1)
		exitWithPerror("listen() error\n" + String(strerror(errno)));
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

	/* init kqueue*/
	int kq;
	kq = kqueue();
	if (kq == -1) { exitWithPerror("kqueue() error\n" + String(strerror(errno))); }
	std::map<int, String> clients; // map clients socket: data
	std::vector<struct kevent> changeList; // kevent vector for changeList
	struct kevent eventList[8];

	/* add event for server socket */
	changeEvents(changeList, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	std::cout << "echo server started" << std::endl;

	/* main loop */
	int newEvents;
	struct kevent * currEvent;
	while(true) {
		/* apply change and return new events(pending events) */
		newEvents = kevent(kq, &changeList[0], changeList.size(), eventList, 8, NULL);
		if (newEvents == -1) { exitWithPerror("kevent() error\n" + String(strerror(errno))); }

		changeList.clear(); // clear for new changes

		for (int i = 0; i < newEvents; ++i) {
			currEvent = &eventList[i];

			/* check error event return  */
			if (currEvent->flags & EV_ERROR) {
				if (currEvent->ident == serverSocket)
					exitWithPerror("server socket error");
				else {
					std::cerr << "clients socket error" << std::endl;
					disconnectClient(currEvent->ident, clients);
				}
			}
			else if (currEvent->filter == EVFILT_READ) {
				if (currEvent->ident == serverSocket) {
					/* Accept new client */
					int clientSocket(accept(serverSocket, NULL, NULL));
					if (clientSocket == -1) { exitWithPerror("accept() error\n" + String(strerror(errno))); }
					std::cout << "accept new client : " << clientSocket << std::endl;
					fcntl(clientSocket, F_SETFL, O_NONBLOCK);

					/* add event for client socket - add read && write event */
					changeEvents(changeList, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					changeEvents(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					clients[clientSocket] = "";
				}
				else if (clients.find(currEvent->ident) != clients.end()) {
					/* read data from clients */
					char buf[1024];
					int n = read(currEvent->ident, buf, sizeof(buf));
					if (n <= 0) {
						if (n < 0) { std::cerr << "clietn read error!" << std::endl; }
						disconnectClient(currEvent->ident, clients);
					}
					else {
						buf[n] = '\0';
						clients[currEvent->ident].append(buf);
						std::cout << "received data from " << currEvent->ident << ":\n" << clients[currEvent->ident] << std::endl;
					}
				}
			}
			else if (currEvent->filter == EVFILT_WRITE) {
				/* send Data to client*/
				std::map<int, String>::iterator it = clients.find(currEvent->ident);
				if (it != clients.end()) {
					int n;
					n = write(currEvent->ident, clients[currEvent->ident].c_str(), clients[currEvent->ident].size());
					if (n == -1) {
						std::cerr << "client write error" << std::endl;
						disconnectClient(currEvent->ident, clients);
					}
					else
						clients[currEvent->ident].clear();
				}
			}
		}
	}
}
