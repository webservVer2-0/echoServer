#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <vector>

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


int main()
{
	int server_socket;
	sockaddr_in server_addr;

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		exit(-1);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);


	std::cout<< "htons " << server_addr.sin_port << std::endl;
	std::cout<< "htonl " << server_addr.sin_addr.s_addr << std::endl;
	std::cout<< "size(sockaddr_in) " << sizeof(sockaddr_in) << std::endl;
	std::cout<< "size(sockaddr) " << sizeof(sockaddr) << std::endl;
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		exit(1);
    if (listen(server_socket, 5) == -1)
        exit(2);

	fcntl(server_socket, F_SETFL, O_NONBLOCK);

	int kq = kqueue();
	if (kq == -1)
		exit(3);

	std::map<int, std::string> clients;
	std::vector<struct kevent> change_list;
	struct kevent event_list[8];

	change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	std::cout << "echo server started" << std::endl;

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
				if (curr_event->ident == server_socket)
					exit(5);
				else
				{
					disconnect_client(curr_event->ident, clients);
				}
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (curr_event->ident == server_socket)
				{
					int client_socket;
					if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
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
						disconnect_client(curr_event->ident, clients);
					}
					else
					{
						buf[n] = '\0';
						clients[curr_event->ident] += buf;
						std::cout << "received data from " << curr_event->ident << ": " << clients[curr_event->ident] << std::endl;
					}
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				std::map<int, std::string>::iterator it = clients.find(curr_event->ident);
				if (it != clients.end())
				{
					if (clients[curr_event->ident] != "")
					{
						int n;
						write(1, clients[curr_event->ident].c_str(), clients[curr_event->ident].size());
						if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(), clients[curr_event->ident].size()) == -1))
						{
							std::cerr << "client write error!" << std::endl;
							disconnect_client(curr_event->ident, clients);
						}
						else
						{
							clients[curr_event->ident].clear();
						}
					}
				}
			}
		}
	}
	return (0);
}