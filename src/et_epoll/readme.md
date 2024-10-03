`g++ echo_server.cpp -o echo_server`

`./echo_server 2223`

不再编写客户端程序，使用`nc`命令模拟一个客户端。

新建一个终端：`nc -v 127.0.0.1 2223`