#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SERV_PORT 6666
#define MAXLINE 100


struct thread_info {
    struct sockaddr_in cliAddr;
    int conn_fd;
};


void *thread_task(void *arg) {
    struct thread_info *data = (struct thread_info*)arg;
    assert(data);
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];

    while (true) {
        int count = read(data->conn_fd, buf, MAXLINE);
        if(count) {
            std::cout << "received from " << inet_ntop(AF_INET, &(*data).cliAddr.sin_addr, str, sizeof(str)) \
            << "Port is " << ntohs((*data).cliAddr.sin_port) << std::endl;
            for (int i = 0 ;i < count ; ++i) {
                buf[i] = toupper(buf[i]);
            }
            write(data->conn_fd, buf, count);
            write(STDOUT_FILENO, buf, count);
        }
    }
}

int main(int argc, char **argv) {

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    int linkfd = socket(AF_INET, SOCK_STREAM, 0);

    bind(linkfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    listen(linkfd, 100);

    socklen_t  cli_addr_len = sizeof(struct sockaddr_in);
    pthread_t tid;

    struct thread_info cli_data[100];
    std::cout << "---------------------------" << std::endl;
    int i = 0;
    while(true) {
        cli_data[i].conn_fd = accept(linkfd, \
            (struct sockaddr*)&cli_data[i].cliAddr, \
            &cli_addr_len);
        pthread_create(&tid, NULL, thread_task, (void *)&cli_data[i]);
        pthread_detach(tid); //不会阻塞主线程
        ++i;
    }
    return 0;
}
