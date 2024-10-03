
1. 水平触发模式：一个事件只要有，就会一直触发
2. 边缘触发模式：只有一个事件从无到有才会触发。

对于服务器端，一般考虑其读内容时的逻辑就好。

如果是水平触发模式，是要输入缓冲中有数据，就会在`epoll_wait`中触发相应的文件描述符，就可以去`read`数据了。这时候读数据不用全部读完，因为“只要有数据就会通知我们去读”，那么我这次多读点、少读点都没关系，数据不会漏读。

但如果是边缘触发模式，输入缓冲中第一次拥有数据的时候通知我们，之后就不会通知了，因此我们需要在通知我们的时候使用循环，把数据全部读走。这样会产生一个问题，因为使用循环去读，最后一个循环时，缓冲里如果没有数据了，`read`应该返回，然后就结束了。但如果是阻塞模式，`read`看到缓冲里没有数据不会返回，只会傻傻地在哪里等待，浪费了事件。因此边缘触发模式需要和非阻塞I/O配合使用。

重点：
1. 理解边缘触发，其实考虑数字电路里那个电平波形就很好理解了。
2. 边缘触发与非阻塞要一起用。关键是理解边缘触发要用一个循环去读数据，可能会出现没东西可读的情况，如果使用阻塞I/O，没东西可读的时候，`read`会阻塞。也就是说，我检测到现在没内容要退出循环了，而阻塞`read`却不愿意返回。

## 边缘模式

1. 使用`errno`变量验证错误原因
2. 更改套接字特性，使用非阻塞I/O

```C++
#include <errno.h>
int errno;//全局变量

#include <fcntl.h>
int fcntl(int filedes,int cmd,...);
//将文件描述符改为非阻塞
int flag=fcntl(fd,F_GETFL,0);
fcntl(fd,F_SETFL,flag|O_NONBLOCK);
```
边缘触发的主要优点：可以分离接收数据和处理数据的时间点。

从实现模型的角度看，边缘触发更有可能带来高性能，但不能简单的认为“只要边缘触发就一定能提高速度”。

`epoll`水平触发和边缘触发运行比较：

令BUF_SIZE=4，分别运行[epoll水平触发]()和[epoll边缘触发]().

结果如下：

水平触发：

客户端：
```sh
lxp@luopan:~/projectCPP/tcpip/src/epoll$ nc -v 127.0.0.1 2222
Connection to 127.0.0.1 2222 port [tcp/*] succeeded!
123
123
123456789
123456789
```
服务器：
```sh
lxp@luopan:~/projectCPP/tcpip/src/epoll$ ./echo_server 2222
epoll wait
connected client:5
epoll wait
epoll wait
epoll wait
epoll wait
```
客户端发送`123`,触发一次`epoll wait`,客户端发送`123456789`,因为9>2*BUF_SIZE,所以触发了3次`epoll wait`

边缘触发：

```sh
lxp@luopan:~/projectCPP/tcpip/src/epoll$ nc -v 127.0.0.1 2222
Connection to 127.0.0.1 2222 port [tcp/*] succeeded!
123
123
1234567890000000
1234567890000000
```
服务器：
```sh
lxp@luopan:~/projectCPP/tcpip/src/epoll$ ./echo_server 2222
epoll wait
connected client:5
epoll wait
epoll wait
```
可以发现，即使客户端发送内容大于BUF_SIZE,也只触发了一次`epoll wait`。
