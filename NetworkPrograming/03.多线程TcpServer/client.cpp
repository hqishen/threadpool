//
// Created by sundae on 2022/5/19.
//

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define SERV_PORT 6666
#define MAXLINE 100

int main(int argc, char **argv) {

    int link_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t servAddrLen = sizeof(servAddr);
    connect(link_fd, (struct sockaddr*)&servAddr, servAddrLen);

    char buf[]="Hello world";
    write(link_fd, buf, strlen(buf));

    int n = read(link_fd, buf, MAXLINE);

    write(STDOUT_FILENO, buf, n);
    close(link_fd);
    return 0;
}