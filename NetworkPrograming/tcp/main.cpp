/*
 *
 */
#include <iostream>
#include <sys/types.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define SERVER_PORT 6666
#define MAXLINE 100

int main() {

    int link_fd = socket(AF_INET, SOCK_STREAM, 0);
//    int connt_fd = 0;
    struct sockaddr_in serAddr, cliAddr;
    char buf[MAXLINE] = {};
    char str[INET_ADDRSTRLEN];
    socklen_t cliAddrLen;

    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(SERVER_PORT);
    serAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(link_fd, (struct sockaddr*)&serAddr, sizeof(serAddr));

    listen(link_fd, 10);
    int cont_fd;

    while(true) {
        cliAddrLen = sizeof(struct sockaddr_in);
        cont_fd = accept(link_fd, (struct sockaddr *)&cliAddr, &cliAddrLen);

       int n = read(cont_fd, buf, MAXLINE);
       std::cout << "receive data len " << n << std::endl;
       std::cout << inet_ntop(AF_INET, &cliAddr.sin_addr, str, sizeof(str)) << " " << ntohs(cliAddr.sin_port) << std::endl;
       for (int i = 0; i < n; ++i) {
           buf[i] = toupper(buf[i]);
       }
       write(cont_fd, buf, n);
       close(cont_fd);

       memset(buf, 0, strlen(buf));
       memset(buf, 0, INET_ADDRSTRLEN);
    }
    close(link_fd);
    return 0;
}
