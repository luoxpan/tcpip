## 半关闭
服务器使用`close`关闭套接字后，可能无法接收到客户端传来的最后信息，所以，需要半关闭状态。例如使用`shutdown`关闭输出流，此时服务器会向客户端发送EOF信息，
客户端接收到信息后，回复服务器"Thank You"，服务器的输入流仍能接收该信息，然后才`close`。

`int shutdown(int sock,int howto)`

`howto`:

* `SHUT_RD`:断开输入流
* `SHUT_WR`:断开输出流
* `SHUT_RDWR`:断开I/O流


## I/O复用技术

复用：为了提高物理设备的效率，用最少的物理要素传递最多数据时使用的技术。

网络编程的I/O复用技术，即只使用一个进程，就可以连接多个客户端，而不需要创建多个进程。

## select
```C++
#include <select.h>
#include <time.h>

int select(int maxfd,fd_set *readset,fd_set *writeset,fd_set *exceptset,const struct timeval *timeout);
// maxfd:监视对象文件描述符数量
// readset:将所有关注“是否存在待读取数据”的文件描述符注册到fd_set，并传递其地址值
// writeset:关注“是否可传输无阻塞数据”的文件描述符集合
// exceptset:关注“是否发生异常”的文件描述符集合
// timeout:传递超时信息，防止无限阻塞
```

`select()`函数可以将多个文件描述符集中到一起进行统一监视：
1. 是否存在套接字接收数据
2. 无需阻塞传输数据的套接字有哪些
3. 哪些套接字发生了异常

上述监视项称为事件(event)。

`select`函数的调用方法和顺序：

步骤一：设置文件描述符；指定监视范围；设置超时

步骤二：调用`select`函数

步骤三：查看调用结果

1. 设置文件描述符，`fd_set`的创建和操作

```C++
int main(void)
{
    fd_set set;
    FD_ZERO(&set);//全部设置为0，表示不监视任何套接字
    FD_SET(1,&set);//fd1=1,表示监视fd1
    FD_SET(2,&set);//fd2=1,表示监视fd2
    FD_CLR(2,&set);//fd2=0,表示不监视fd2
}
```

2. 设置监视范围和超时

```C++
struct timeval
{
    long tv_sec; //seconds
    long tv_usec;//microseconds
}
```

3. 调用`select`后查看返回结果

若`select`的返回值为大于0的整数，说明相应数量的文件描述符发生了变化。

此时，`fd_set`中仍为1的描述符为发生了事件的文件。例如调用`select`后`readset`中为`00101`，说明`fd2,fd4`等待读取（有读取事件）





