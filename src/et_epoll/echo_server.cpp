#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

const int BUF_SIZE = 4;
const int EPOLL_SIZE = 50;
void error_handling(const char *message)
{
    std::cerr << message << std::endl;
    exit(1);
}

void set_nonblocking_mode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        error_handling("Need port for ./server");
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("socket() error");
    }
    // 1. serv_sock改为非阻塞
    set_nonblocking_mode(serv_sock);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_handling("listen() error");
    }

    int epfd = epoll_create(EPOLL_SIZE);
    if (epfd == -1)
    {
        error_handling("epoll_create() error");
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event) == -1)
    {
        error_handling("epoll_ctl() for serv_sock error");
    }

    char buf[BUF_SIZE];
    struct epoll_event *ep_events = new struct epoll_event[EPOLL_SIZE];
    while (true)
    {
        int event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            error_handling("epoll_wait() error");
        }
        puts("epoll wait");
        for (int i = 0; i < event_cnt; i++)
        {
            if (ep_events[i].data.fd == serv_sock)
            {
                struct sockaddr_in clnt_addr;
                socklen_t adr_size = sizeof(clnt_addr);
                int clnt_sock = accept(serv_sock, (struct sockaddr *)(&clnt_addr), &adr_size);
                if (clnt_sock == -1)
                {
                    error_handling("accept() error");
                }
                set_nonblocking_mode(clnt_sock);
                // 2. 新加的与客户端连接的套接字加上边缘触发
                // event.events = EPOLLIN;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clnt_sock;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event) == -1)
                {
                    error_handling("epoll_ctl() for clnt_sock error()");
                }
                printf("connected client:%d\n", clnt_sock);
            }
            else
            {
                // ET 模式需要循环读取
                while (true)
                {
                    int str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                    if (str_len == 0)
                    {
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, nullptr) == -1)
                        {
                            error_handling("epoll_ctl() for i.data.fd error");
                        }
                        close(ep_events[i].data.fd);
                        printf("colsed client:%d\n", ep_events[i].data.fd);
                        break; // 边缘模式，退出while循环
                    }
                    else if (str_len < 0)
                    {
                        if (errno == EAGAIN) // 读到没有数据可读
                            break;
                    }
                    else
                    {
                        write(ep_events[i].data.fd, buf, str_len);
                    }
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}