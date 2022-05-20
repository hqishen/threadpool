//
// Created by sundae on 2022/5/20.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, const char ** argv) {

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_t servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);

    connect(sock_fd, (struct sockaddr*)&servAddr, sizeof(servAddr));

    char buf[] = "Hello Server";
    write(sock_fd, buf, strlen(buf));
    int n = read(sock_fd, buf, 100);
    write(STDOUT_FILENO, buf, n);
    close(sock_fd);


    return 0;
}

