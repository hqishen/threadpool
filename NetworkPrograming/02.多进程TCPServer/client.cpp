//
// Created by sundae on 2022/5/19.
//
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SERV_PORT 8888
#define MAXLINE 1000


int main(int argc, char **argv) {

    struct sockaddr_in serAddr;

    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serAddr.sin_addr);
    int connFD = socket(AF_INET, SOCK_STREAM, 0);
    char buf[MAXLINE] = "Hello World";
    char str[INET_ADDRSTRLEN];

    while(true) {
        connect(connFD, (struct sockaddr*)&serAddr, sizeof(serAddr));
        write(connFD, buf, MAXLINE);
        int count = read(connFD, buf, MAXLINE);
        write(STDOUT_FILENO, buf, count);
        break;
//        memset()
    }
    close(connFD);
    return 0;
}