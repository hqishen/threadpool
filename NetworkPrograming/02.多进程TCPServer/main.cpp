
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 100
#define SERV_PORT 8888

void signal_task(int num) {
    while(waitpid(0, NULL, WNOHANG) > 0);
}


int main() {
    struct sockaddr_in servAddr, cliAddr;
    socklen_t clientAddrLen = sizeof(cliAddr);
    int linkFD = socket(AF_INET, SOCK_STREAM, 0); //创建一个socket
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(linkFD, (struct sockaddr*)&servAddr, sizeof(servAddr));
    listen(linkFD, 10); // 可以同时监听10个端


    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    pid_t pid;

    struct sigaction sig_cat;
    sig_cat.sa_flags = 0;
    sig_cat.sa_handler = signal_task;
    sigemptyset(&sig_cat.sa_mask);
    sigaction(SIGCHLD, &sig_cat, NULL);
    int opt = 1;
    setsockopt(linkFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::cout << "开始接收。。。" << std::endl;
    while(1) {
        int connFD = accept(linkFD, (struct sockaddr *)&cliAddr, &clientAddrLen);
        pid = fork();
        if (pid == 0) { //是子进程
            close(linkFD); // 在子进程不要打开连接socket
            while(true) {
                int n = read(connFD, buf, MAXLINE);
                if (n == 0) {
                    perror("read data error");
                    break;
                }
                // 读到消息
                printf("received from %s at PORT %d\n",
                       inet_ntop(AF_INET, &cliAddr.sin_addr, str, sizeof(str)),
                       ntohs(cliAddr.sin_port));
                for (int i = 0; i < n; i++)
                    buf[i] = toupper(buf[i]);
                write(connFD, buf, n);
            }

            close(connFD);
            return 0; //子进程退出
        } else if (pid > 0) { // 父进程
            close(connFD); // 在父进程不要打开连接socket
        }
    }
    return 0;
}
