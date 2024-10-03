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

    FD_ISSET(3,&set);//fd3是否发生事件
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


从[select服务器](https://github.com/luoxpan/tcpip/tree/main/src/select)代码中不难看出基于`select`的I/O复用技术存在的问题：

1. 调用`select`函数后针对所有文件描述符的循环语句
2. 每次调用`select`函数时都需要向该函数传递监视对象信息

循环语句很好理解，但`select`真正的性能瓶颈往往是第二项，从函数调用来看，我们似乎只是将`fd_set`的地址传给`select`，然而，由于操作的对象是文件描述符，而文件描述符是由操作系统管理的，因此每次调用`select`都需要将`fd_set`从用户态复制到内核态，`select`返回时又需要从内核态复制到用户态，这在高并发情况下会造成性能瓶颈。

除此之外，`select`通常有一个文件描述符数量的限制（在大多数系统上是1024）。`select`通过轮询收集文件描述符的事件信息，这种轮询在大量文件描述符处于非活动状态时浪费CPU资源。

`poll`没有文件描述符数量限制，但并没有解决`select`复制和轮询的问题。

`select`的优点：平台兼容性

## epoll
```C++
#include <sys/epoll.h>
```
**`epoll`的优点**：
1. 无需编写以监视状态变化为目的的针对所有文件描述符的循环语句
2. 调用对应于`select`函数的`epoll_wait`函数时无需再次传递监视对象信息

`epoll_create`:创建保存epoll文件描述符的空间

`epoll_ctl`:向空间注册或注销文件描述符

`epoll_wait`:与select函数类似，等待文件描述符发生变化

为了解决`select`重复复制的问题，`epoll`向操作系统请求创建保存文件描述符的空间，通过`epoll_ctl`请求操作系统完成文件描述符的添加和删除，使用`epoll_wait`等待文件描述符的变化。

`select`通过`fd_set`变量查看监视对象的状态变化，而`epoll`通过`epoll_event`结构体将发生事件的文件描述符集中到一起。

```C++
struct epoll_event
{
    __uint32_t events;
    epoll_data_t data;
}

typedef union epoll_data
{
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint62_t u64;
}epoll_data_t;
```
声明足够大的`epoll_event`数组后，传递给`epoll_wait`函数时，发生变化的文件描述符信息将填入该数组，无需像`select`函数那样对所有文件描述符进行循环。

```C++
int epoll_create(int size);
```
size:epoll实例的大小，较新的linux内核会忽视size传入，自动调整epoll实例大小

成功返回epoll文件描述符，失败返回-1

```C++
int epoll_ctl(int epfd,int op,int fd,struct epoll_event * event);
```
epfd:epoll例程的文件描述符

op  :用于指定监视对象的添加、删除或更改等操作

* `EPOLL_ADD`,`EPOLL_DEL`,`EPOLL_MOD` 

fd  :需要注册的监视对象文件描述符

event:监视对象的事件类型
```C++
struct epoll_event event;
event.events=EPOLLIN;
event.data.fd=sockfd;
epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);
```
`event.events`:
* `EPOLLIN`:需要读取数据的情况
* `EPOLLOUT`:输出缓冲为空，可以立即发送数据的情况
* `EPOLLPRI`:收到OOB数据的情况
* `EPOLLRDHUP`:断开连接或半关闭的情况，在边缘触发模式下很有用
* `EPOLLERR`:发生错误的情况
* `EPOLLER`:以边缘触发的方式得到事件通知
* `EPOLLONESHOT`:发生一次事件后，相应文件描述符不再收到事件通知，需要向`epoll_ctl`函数的第二个参数传递`EPOLL_CTL_MOD`，再次设置事件。







