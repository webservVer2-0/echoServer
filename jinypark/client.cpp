#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <cstring>

int main(int ac, char** av)
{
    int sockfd;
    struct sockaddr_in servaddr;
    char sendline[1024], recvline[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(av[1]));
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        std::cerr << "connection error!" << std::endl;
        exit(1);
    }

    while (fgets(sendline, 1024, stdin) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));
        if (read(sockfd, recvline, 1024) == 0)
        {
            std::cerr << "server terminated prematurely!" << std::endl;
            exit(2);
        }

        std::cout << "Echo from server: " << recvline;
        memset(recvline, 0, 1024);
    }

    close(sockfd);
    return 0;
}
