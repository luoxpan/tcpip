
服务器端基本四步：
1. 调用`socket`函数创建套接字 
2. 调用`bind`函数分配IP地址和端口号
3. 调用`listen`函数转为可接收请求状态
4. 调用`accept`函数受理连接请求

客户端：
1. 调用`socket`函数创建套接字
2. 调用`connect`函数向服务器发送连接请求

```C++
int socket(int domain,int type,int protocol);

int bind(int sockfd,struct sockaddr *myaddr,socklen_t addrlen);

int listen(int sockfd,int backlog);

int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen);

int connect(int sockfd,struct sockaddr* serv_addr,socklen_t addrlen);
```

常用库函数所在文件：
```C++
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>//ubuntu中没有<sys/inet.h>
#include <arpa/inet.h>
#include <unistd.h> // for close()
```



## socket
套接字：套接字是应用程序与网络的接口，允许程序发送和接收数据。称为套接字是因为他像水管接头一样，将不同的网络应用连接到其他设备。

```C++
int socket(int domain,int type,int protocol);
```

`domain`(协议簇):`PF_INET`:IPv4,`PF_INET6`:IPv6

`type`(套接字类型):`SOCK_STREAM`:TCP,`SOCK_DGRAM`:UDP

`SOCK_STREAM`:可靠的、按序传递的、基于字节的面向连接的数据传输方式的套接字

`SOCK_DGRAM`:不可靠的、不按序传递的、以数据的高速传输为目的的套接字

`protocol`:一般为0，除非遇到“同一协议簇中存在多个传输方式相同的协议”

## bind

`bind`函数为创建的套接字绑定地址信息，地址信息由IP和端口号组成，前者区分不同主体，后者区分同一主机中的不同应用程序。

```C++
int bind(int sockfd,struct sockddr* myaddr,socklen_t addrlen);

struct sockaddr
{
    sa_family_t     sin_family;     //地址族
    char            sa_data[14];    //地址信息
}
```
`bind`并非只为IPv4设计，`bind`要求的`struct sockaddr`对于IPv4的地址填充来说太麻烦，因此有了IPv4地址的结构体`struct sockaddr_in`，我们构建`sockaddr_in`结构体，然后转换为`sockaddr`结构体即可。

1. in 是internet的缩写
2. sin 是socket internet native，表示用于套接字的、基于互联网的、以原生格式存储地址信息的结构体

```C++
struct sockaddr_in
{
    sa_family_t     sin_family; //地址族
    unit16_t        sin_port;   //16位TCP/UDP端口号
    struct in_addr  sin_addr;   //32位IP地址
    char            sin_zero[8];//不使用
}

struct in_addr
{
    in_addr_t       s_addr;     //32位IPv4地址
}
```
`sin_family`:`AF_INET`,`AF_INET6`

`sin_port`:网络字节序为大端序，CPU大多为小端序。因此`sin_port`参数传入需要使用`htons`进行字节序转换。

1. h表示主机(host),n表示网络(network)
2. `htons`：把short型数据从主机字节序转换为网络字节序。`ntohs`则相反
3. `htonl`: 把long型数据从主机字节序转换为网络字节序。`ntohl`则相反
4. `htons`用于端口号转换，`htonl`用于IP地址转换。`htons(port);htonl(INADDR_ANY)`

`sin_addr`: 我们一般习惯使用点分十进制格式字符串表示ip地址，但`sin_addr.s_addr`要求32位整数。

两种做法，后者更常用，两种做法都自动帮我们完成了字节序转换，因此不需要再调用`htonl`
1. `inet_addr()`：无法区分`255.255.255.255`和其他不合规ip错误
```C++
#include <arpa/inet.h>
//in_addr_t inet_addr(const char* string);
char *ip_addr="1.2.3.4"
unsigned long conv_addr=inet_addr(ip_addr);
if(conv_addr==INADDR_NONE) std::cerr<<"error\n";
```
2. `inet_aton` :错误处理优于`inet_addr`，更常用。
```C++
#include <arpa/inet.h>
//int inet_aton(const char* string ,struct in_addr*addr);
char *ip_addr="1.2.3.4"
struct sockaddr_in addr_inet;
if(!inet_aton(ip_addr,&addr_inet.sin_addr)) std::cerr<<"error\n";
```

`sin_zero`:无特殊含义，只是为了使结构体`sockaddr_in`的大小与`sockaddr`结构体保持一致而插入的成员，必须填充为0

综上IPv4网络地址初始化过程如下：
```C++
struct sockaddr_in addr;
memset(&addr,0,sizeof(addr));
char *serv_ip="127.123.100.147";
char *serv_port="9190";
addr.sin_family=AF_INET;
addr.sin_port=htons(atoi(serv_port));
if(!inet_aton(serve_ip,&addr.sin_addr)) std::cerr<<"error\n";
//addr.sin_addr.s_addr=inet_addr(serve_ip);
```

最后，对于服务器，采用ip地址硬编码并不是一个好的选择，这意味着主机ip地址改变时需要手动修改代码，且如果计算机有多个IP地址，则只要端口号一致，就可以从不同IP地址接收数据，采用硬编码将无法实现该好处。

使用`INADDR_ANY`表示绑定到任意地址，自动获取运行服务器端的计算机IP地址，无需亲自输入。

```C++
struct sockaddr_in addr;
memset(&addr,0,sizeof(addr));
char *serv_port="9190";
addr.sin_family=AF_INET;
addr.sin_port=htons(atoi(serv_port));
addr.sin_addr.s_addr=htonl(INADDR_ANY);
```

## listen

调用`bind`函数给套接字分配了地址，接下来就要通过调用`listen`函数进入等待连接请求状态。

```C++
int listen(int sockfd,int backlog);
// sockfd 套接字文件描述符
// backlog 连接请求等待队列的大小
```
`listen`像一个门卫，接受到连接请求时，将请求放入等候室。

## accept

调用`listen`后，若有新的连接请求，则应按序受理，受理请求本质上就是读写套接字或者说文件，在linux里，套接字和文件没有本质区别，只是我们在进行网络编程时习惯将其称为套接字罢了。

使用`accept`受理请求则会自动创建一个套接字，并自动与发起连接请求的客户端建立连接。

```C++
int accept(int sock,struct sockaddr *addr,socklen_t *addr_len);
```