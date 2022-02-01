#include "socketsOps.h"
#include "net_endian.h"
#include "async_log.h"
#include <cstring>

BEGIN_NS(net)

const struct sockaddr* SocketsOps::sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

struct sockaddr* SocketsOps::sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

const struct sockaddr_in* SocketsOps::sockaddr_in_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
}

struct sockaddr_in* SocketsOps::sockaddr_in_cast(struct sockaddr* addr)
{
    return static_cast<struct sockaddr_in*>(static_cast<void*>(addr));
}

SOCKET SocketsOps::createOrDie()
{
#ifdef WIN32
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOGF("SocketsOps::createNonblockingOrDie");
    }

    int err = GetLastError();
#else
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOGF("SocketsOps::createNonblockingOrDie");
    }
#endif

    return sockfd;
}

SOCKET SocketsOps::createNonblockingOrDie()
{
#ifdef WIN32
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOGF("SocketsOps::createNonblockingOrDie");
    }
#else
    SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOGF("SocketsOps::createNonblockingOrDie");
    }
#endif

    setNonBlockAndCloseOnExec(sockfd);
    return sockfd;
}

void SocketsOps::setNonBlockAndCloseOnExec(SOCKET sockfd)
{
#ifdef WIN32
    //将socket设置成非阻塞的
    unsigned long on = 1;
    ::ioctlsocket(sockfd, FIONBIO, &on);
#else
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    // FIXME check

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
    // FIXME check

    (void)ret;
#endif       
}

void SocketsOps::bindOrDie(SOCKET sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
    if (ret < 0)
    {
        LOGF("SocketsOps::bindOrDie");
    }
}

void SocketsOps::listenOrDie(SOCKET sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        LOGF("SocketsOps::listenOrDie");
    }
}

SOCKET SocketsOps::accept(SOCKET sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof * addr);
#ifdef WIN32
    SOCKET connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else  
    SOCKET connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0)
    {
#ifdef WIN32
        int savedErrno = ::WSAGetLastError();
        LOGSYSE("Socket::accept");
        if (savedErrno != WSAEWOULDBLOCK)
            LOGF("unexpected error of ::accept %d", savedErrno);
#else
        int savedErrno = errno;
        LOGSYSE("Socket::accept");
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            LOGF("unexpected error of ::accept %d", savedErrno);
            break;
        default:
            LOGF("unknown error of ::accept %d", savedErrno);
            break;
        }

#endif
    }

    return connfd;
}

void SocketsOps::setReuseAddr(SOCKET sockfd, bool on)
{
    int optval = on ? 1 : 0;
#ifdef WIN32
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
#else
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
#endif
    if (ret < 0)
    {
        LOGF("SocketsOps::setReuseAddr");
    }
    // FIXME CHECK
}

void SocketsOps::setReusePort(SOCKET sockfd, bool on)
{
    //Windows 系统没有 SO_REUSEPORT 选项
#ifndef WIN32
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOGSYSE("SO_REUSEPORT failed.");
    }
#endif
}

SOCKET SocketsOps::connect(SOCKET sockfd, const struct sockaddr_in& addr)
{
    return ::connect(sockfd, sockaddr_cast(&addr), static_cast<socklen_t>(sizeof addr));
}

int32_t SocketsOps::read(SOCKET sockfd, void* buf, int32_t count)
{
#ifdef WIN32
    return ::recv(sockfd, (char*)buf, count, 0);
#else
    return ::read(sockfd, buf, count);
#endif
}

#ifndef WIN32
ssize_t SocketsOps::readv(SOCKET sockfd, const struct iovec* iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}
#endif

int32_t SocketsOps::write(SOCKET sockfd, const void* buf, int32_t count)
{
#ifdef WIN32
    return ::send(sockfd, (const char*)buf, count, 0);
#else
    return ::write(sockfd, buf, count);
#endif

}

void SocketsOps::close(SOCKET sockfd)
{
#ifdef WIN32   
    if (::closesocket(sockfd) < 0)
#else
    if (::close(sockfd) < 0)
#endif
    {
        LOGSYSE("SocketsOps::close, fd=%d, errno=%d, errorinfo=%s", sockfd, errno, strerror(errno));
    }
}

void SocketsOps::shutdownWrite(SOCKET sockfd)
{
#ifdef WIN32
    if (::shutdown(sockfd, SD_SEND) < 0)
#else
    if (::shutdown(sockfd, SHUT_WR) < 0)
#endif        
    {
        LOGSYSE("SocketsOps::shutdownWrite");
    }
}

void SocketsOps::toIpPort(char* buf, size_t size, const struct sockaddr_in& addr)
{
    ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
    size_t end = ::strlen(buf);
    uint16_t port = sockets::networkToHost16(addr.sin_port);

    snprintf(buf + end, size - end, ":%u", port);
}

void SocketsOps::toIp(char* buf, size_t size, const struct sockaddr_in& addr)
{
    if (size >= sizeof(struct sockaddr_in))
        return;

    ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}

void SocketsOps::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = sockets::hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOGSYSE("SocketsOps::fromIpPort");
    }
}

int SocketsOps::getSocketError(SOCKET sockfd)
{
    int optval;
#ifdef WIN32
    int optvallen = sizeof(optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optvallen) < 0)
        return ::WSAGetLastError();
#else
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        return errno;
#endif
    return optval;
}

struct sockaddr_in SocketsOps::getLocalAddr(SOCKET sockfd)
{
    struct sockaddr_in localaddr = { 0 };
    memset(&localaddr, 0, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    ::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen);
    return localaddr;
}

struct sockaddr_in SocketsOps::getPeerAddr(SOCKET sockfd)
{
    struct sockaddr_in peeraddr = { 0 };
    memset(&peeraddr, 0, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    ::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen);
    return peeraddr;
}

bool SocketsOps::isSelfConnect(SOCKET sockfd)
{
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

END_NS(net)
