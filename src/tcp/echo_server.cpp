#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BUF_SIZE = 1024;

void error_handling(const char *message)
{
    fputs(message, stderr);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        error_handling("Need port for ./server");
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket error");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if (bind(serv_sock, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) == -1)
    {
        error_handling("bind error");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_handling("listen error");
    }

    char message[BUF_SIZE];
    for (int i = 0; i < 5; i++)
    {
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_sz = sizeof(clnt_addr);
        int clnt_sock = accept(serv_sock, (struct sockaddr *)(&clnt_addr), &clnt_addr_sz);
        if (clnt_sock == -1)
        {
            error_handling("accept error");
        }
        else
        {
            printf("Connected client %d \n", i + 1);
        }
        int rstr_len = 0;
        while ((rstr_len = read(clnt_sock, message, BUF_SIZE)) != 0)
        {
            write(clnt_sock, message, rstr_len);
        }
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}