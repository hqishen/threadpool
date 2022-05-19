//
// Created by sundae on 2022/5/18.
//

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 100
#define SERV_PORT 6666


int main(int argc, const char ** argv) {

    int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
//    setsockopt()
    struct sockaddr_in serAddr;
    bzero(&serAddr, sizeof(serAddr));

    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serAddr.sin_addr);

    char buf[MAXLINE];
    while(true) {
        char *str = "Hello word";
        connect(cli_fd, (struct sockaddr*)&serAddr, sizeof(serAddr));
        write(cli_fd, str, sizeof(str));
        int n = read(cli_fd, buf, MAXLINE);
        std::cout << "Read data:" << buf << std::endl;
        write(STDOUT_FILENO, buf, n);
        close(cli_fd);
    }
    return  0;
}