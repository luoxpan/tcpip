#include <iostream>
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
    if (argc != 2)
    {
        std::cerr << "port: " << argv[0] << std::endl;
        exit(1);
    }
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("socket() error");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(serv_sock, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_handling("listen() error");
    }

    struct sockaddr_in clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, reinterpret_cast<struct sockaddr *>(&clnt_addr), &clnt_addr_size);
    if (clnt_sock == -1)
    {
        error_handling("accept() error");
    }

    char message[] = "Hello World!";
    write(clnt_sock, message, sizeof(message));
    close(clnt_sock);
    close(serv_sock);
    return 0;
}