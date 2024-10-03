#include <iostream>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#include <memory>
#include <unistd.h>
#include <arpa/inet.h>

const int BUF_SIZE = 1024;
void error_handling(const char *message)
{
    std::cerr << message << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        error_handling("Need port for ./server");
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);

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

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(serv_sock, &readset);
    int fd_max = serv_sock;

    struct timeval timeout;
    char buf[BUF_SIZE];
    while (true)
    {
        fd_set cpy_rset = readset;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        int fd_num = select(fd_max + 1, &cpy_rset, nullptr, nullptr, &timeout);
        if (fd_num == -1)
            break;
        if (fd_num == 0)
            continue;
        for (int i = 0; i < fd_max + 1; i++)
        {
            if (FD_ISSET(i, &cpy_rset))
            {
                if (i == serv_sock) // 连接请求
                {
                    struct sockaddr_in clnt_adr;
                    socklen_t adr_sz = sizeof(clnt_adr);
                    int clnt_sock = accept(serv_sock, (struct sockaddr *)(&clnt_adr), &adr_sz);
                    FD_SET(clnt_sock, &readset);
                    if (fd_max < clnt_sock)
                    {
                        fd_max = clnt_sock;
                    }
                    printf("connected client:%d\n", clnt_sock);
                }
                else
                { // 读消息
                    int str_len = read(i, buf, BUF_SIZE);
                    if (str_len == 0) // 关闭请求
                    {
                        FD_CLR(i, &readset);
                        close(i);
                        printf("closed client:%d\n", i);
                    }
                    else
                    {
                        write(i, buf, str_len);
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}