#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>

class Myserver
{
private:
public:
	int _port;
	int _server_socket;
	int _client_socket;
	sockaddr_in _server_addr;
	std::string _msg;
	Myserver(int port, std::string msg) : _port(port), _msg(msg)
	{
		if ((_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			exit(-1);
		std::cout << "socket " << _server_socket << std::endl;
		memset(&_server_addr, 0, sizeof(_server_addr));
		_server_addr.sin_family = AF_INET;
		_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		_server_addr.sin_port = htons(port);
		std::cout<< "port: " << ntohs(_server_addr.sin_port) << std::endl;
		std::cout<< "addr: " << _server_addr.sin_addr.s_addr << std::endl;
		if (bind(_server_socket, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1)
			exit(1);
		if (listen(_server_socket, 10) == -1)
			exit(2);
		fcntl(_server_socket, F_SETFL, O_NONBLOCK);
		msg.insert(_msg.find(']'), "hi123");
	}
	~Myserver()
	{
		// close(_server_socket);
	}
};

void change_events(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}


void disconnect_client(int client_fd, std::map<int, std::string>& clients)
{
    std::cout << "client disconnected: " << client_fd << std::endl;
    close(client_fd);
    clients.erase(client_fd);
}


int main(int ac, char** av)
{
	std::ifstream httpmsg("./httpmsg");
	std::ifstream entity("./index.html");

	std::string msg1;
	std::string msg2;

	std::getline(httpmsg, msg1, '\0');
	std::getline(entity, msg2, '\0');
	msg1 += (char)13;
	msg1 += "\n";
	msg1 += "\n";
	msg1 += msg2;

	int kq = kqueue();
	if (kq == -1)
		exit(3);
	std::map<int, Myserver> server_list;
	std::map<int, std::string> clients;
	std::vector<struct kevent> change_list;
	struct kevent event_list[8];

	for (int i = 1; i < ac; ++i)
		server_list.insert(std::pair<int, Myserver>(atoi(av[i]), Myserver(atoi(av[i]), msg1)));
	for (std::map<int, Myserver>::iterator it = server_list.begin(); it != server_list.end(); ++it)
	{
		change_events(change_list, (*it).second._server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}
	std::cout << "multi server started" << std::endl;

	int new_events;
	struct kevent* curr_event;
	while(1)
	{
		new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		if (new_events == -1)
			exit(4);

		change_list.clear();
		for (int i = 0; i < new_events; ++i)
		{
			curr_event = &event_list[i];
			if (curr_event->flags & EV_ERROR)
			{
				std::map<int, Myserver>::iterator it = server_list.find(curr_event->ident);
				if (it != server_list.end())
					exit(5);
				else
				{
					disconnect_client(curr_event->ident, clients);
				}
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				std::map<int, Myserver>::iterator it = server_list.begin();
				for (; it != server_list.end(); ++it)
				{
						if (curr_event->ident == (*it).second._server_socket)
						{
							break;
						}
				}
				if (it != server_list.end())
				{
					int client_socket;
					if ((client_socket = accept((*it).second._server_socket, NULL, NULL)) == -1)
						exit(6);
					std::cout << "accept new client " << client_socket << std::endl;
					fcntl(client_socket, F_SETFL, O_NONBLOCK);

					change_events(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					clients[client_socket] = "";
				}
				else if (clients.find(curr_event->ident) != clients.end())
				{
					char buf[1024];
					int n = read(curr_event->ident, buf, sizeof(buf));
					if (n <= 0)
					{
						if (n < 0)
							std::cerr << "client read error!" << std::endl;
						std::cout << "read done\n";
						clients[curr_event->ident].clear();
						// disconnect_client(curr_event->ident, clients);
					}
					else
					{
						buf[n] = '\0';
						clients[curr_event->ident] += buf;
						std::cout << "received data from " << curr_event->ident << ": " << clients[curr_event->ident] << std::endl;
						std::cout << curr_event->ident << std::endl;
					}
				}
				std::map<int, std::string>::iterator itt = clients.find(curr_event->ident);
				if (itt != clients.end())
				{
					if (clients[curr_event->ident] != "")
					{
						int n;
						if ((n = write(curr_event->ident, msg1.c_str(), msg1.size()) == -1))
						{
							std::cerr << "client write error!" << std::endl;
							disconnect_client(curr_event->ident, clients);
						}
						else
						{
							std::cout << "success!" << std::endl;
							clients[curr_event->ident].clear();
						}
					}
					// else
					// {
					// 	std::cout << curr_event->ident << std::endl;
					// 	std::cout << clients[curr_event->ident] << std::endl;
					// }
				}
			}
		}
	}
	return (0);
}