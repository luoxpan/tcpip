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
}