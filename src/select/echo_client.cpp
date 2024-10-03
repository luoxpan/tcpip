#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUF_SIZE = 1024;

void error_handling(const char *message)
{
    fputs(message, stderr);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        error_handling("Need ip_addr and port for ./client");
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    if (connect(sock, (struct sockaddr *)(&addr), sizeof(addr)) == -1)
    {
        error_handling("connect error");
    }
    else
    {
        puts("Connected........");
    }

    char message[BUF_SIZE];
    while (1)
    {
        fputs("Input message(Q to quit):", stdout);
        fgets(message, BUF_SIZE, stdin);
        if (strcmp(message, "Q\n") == 0 || strcmp(message, "q\n") == 0)
            break;
        int wstr_len = write(sock, message, strlen(message));
        int recv_len = 0;
        while (recv_len < wstr_len)
        {
            int recv_cnt = read(sock, &message[recv_len], BUF_SIZE - 1);
            if (recv_cnt == -1)
                error_handling("read() error");
            recv_len += recv_cnt;
        }
        message[recv_len] = 0;
        printf("Message from server:%s", message);
    }
    close(sock);

    return 0;
}