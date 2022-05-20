#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERV_NUM 8888
#define NUMBER 50


int main() {
    struct sockaddr_in servAddr;
    int opt = 1;

    int link_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(link_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_NUM);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(link_fd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    listen(link_fd, 200);

    int client_fd[FD_SETSIZE];
    memset(client_fd, -1, FD_SETSIZE * sizeof(int));

    int max_fd = link_fd;
    char buf[BUFSIZ], str[INET_ADDRSTRLEN]; //存buf
    fd_set fd_sets, r_set;
    FD_ZERO(&fd_sets);
    FD_SET(link_fd, &fd_sets); // 把link_fd 加入到set集合中

    struct sockaddr_in cliAddr;
    int cli_fd, sock_fd, ready = 0, max_index = 0;

    while(true) {
        r_set = fd_sets;
        ready = select(max_fd + 1, &r_set, NULL, NULL, NULL); //读

        if (ready < 0) {
            perror("select error");
        }

        if (FD_ISSET(link_fd, &r_set)) { // 有客户端连接
            socklen_t cliAddrLen = sizeof(cliAddr);
            cli_fd = accept(link_fd, (struct sockaddr*)&cliAddr, &cliAddrLen);
            printf("Received from %s at Port :%d \n", \
                inet_ntop(AF_INET, &cliAddr.sin_addr, str, sizeof(str)), \
                ntohs(cliAddr.sin_port));
            int index = 0;
            for ( int i = 0; i < FD_SETSIZE; ++i) {
                if (client_fd[i] < 0) {
                    client_fd[i] = cli_fd;
                    index = i;
                    break;
                }
            }

            if (index == FD_SETSIZE) {
                perror("too many clients");
//                exit()
            }

            FD_SET(cli_fd, &fd_sets);
            if (cli_fd > max_fd) {
                max_fd = cli_fd;
            }
            if (index > max_index) {
                max_index = index;
            }

            if (--ready == 0) {
                continue;
            }
        }

        for (int i = 0; i <= max_index; ++i) {
            if ((sock_fd = client_fd[i]) < 0) {
                continue;
            }

            if (FD_ISSET(sock_fd, &r_set)) {
                int n = read(sock_fd, buf, sizeof(buf));
                if (n == 0) { // 当client关闭连接时，服务器也关闭对应连接
                    close(sock_fd);
                    FD_CLR(sock_fd, &fd_sets);
                    client_fd[i] = -1;
                } else if (n > 0) {
                    for(int j = 0; j < n ; ++j) {
                        buf[j] = toupper(buf[j]);
                    }
                    write(sock_fd, buf, n);
                    write(STDOUT_FILENO, buf,n);
                }
                if (--ready == 0) {
                    break;
                }
            }
        }
    }

    close(link_fd);
    return 0;
}
