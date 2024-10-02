#include <iostream>
#include <cstring>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // for close()

void error_handling(const char *message)
{
    std::cerr << message << std::endl;
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        error_handling("Need ip_addr and port for ./client");
    }
    int clnt_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1)
    {
        error_handling("socket() error");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (connect(clnt_sock, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) == -1)
    {
        error_handling("connect() error");
    }

    char message[30];
    int rstr_len = read(clnt_sock, message, sizeof(message) - 1);
    if (rstr_len == -1)
    {
        error_handling("read() error");
    }
    printf("Message from server:%s\n", message);
    close(clnt_sock);
    return 0;
}