#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif

#ifdef WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Shlwapi.lib")

//remove warning C4996 on Windows
//#define _CRT_SECURE_NO_WARNINGS

typedef int          socklen_t;
//typedef uint64_t     ssize_t;
typedef unsigned int in_addr_t;

//Windows 上没有这些结构的定义，为了移植方便，手动定义这些结构
#define  XPOLLIN         1         //表示对应的文件描述字可以读；
#define  XPOLLPRI        2
#define  XPOLLOUT        4         //表示对应的文件描述字可以写；
#define  XPOLLERR        8 
#define  XPOLLHUP        16        //表示对应的文件描述字被挂起；
#define  XPOLLNVAL       32
#define  XPOLLRDHUP      8192      //表示套接字的一端已经关闭，或者半关闭；

#define  XEPOLL_CTL_ADD  1
#define  XEPOLL_CTL_DEL  2
#define  XEPOLL_CTL_MOD  3

#pragma pack(push, 1)
typedef union epoll_data {
    void*    ptr;
    int      fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;    /* Epoll events */
    epoll_data_t data;      /* User data variable */
};
#pragma pack(pop)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <Ws2ipdef.h>
#include <io.h>     //_pipe
#include <fcntl.h>  //for O_BINARY
#include <shlwapi.h>
#include "common.h"

class BASE_API NetworkInitializer
{
public:
    NetworkInitializer();
    ~NetworkInitializer();
};

#else

typedef int SOCKET;

#define SOCKET_ERROR -1

#define closesocket(s) close(s)


#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>
#include <stdint.h>
#include <endian.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>
#include <errno.h>
#include <dirent.h>


#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <sys/syscall.h>

//for ubuntu readv not found
#ifdef __UBUNTU
#include <sys/uio.h>
#endif 


#define  XPOLLIN         POLLIN
#define  XPOLLPRI        POLLPRI
#define  XPOLLOUT        POLLOUT
#define  XPOLLERR        POLLERR 
#define  XPOLLHUP        POLLHUP
#define  XPOLLNVAL       POLLNVAL
#define  XPOLLRDHUP      POLLRDHUP

#define  XEPOLL_CTL_ADD  EPOLL_CTL_ADD
#define  XEPOLL_CTL_DEL  EPOLL_CTL_DEL
#define  XEPOLL_CTL_MOD  EPOLL_CTL_MOD

//Linux下没有这两个函数，定义之
#define ntohll(x) be64toh(x)
#define htonll(x) htobe64(x)

#endif

#endif