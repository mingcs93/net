#ifndef __NET_SOCKETSOPS_H
#define __NET_SOCKETSOPS_H

#include "common.h"
#include "platform.h"

BEGIN_NS(net)

class SocketsOps
{
public:
	///
	/// Creates a socket file descriptor,
	/// abort if any error.
	static SOCKET createOrDie();

	static SOCKET createNonblockingOrDie();

	static void setNonBlockAndCloseOnExec(SOCKET sockfd);

	static void setReuseAddr(SOCKET sockfd, bool on);

	static void setReusePort(SOCKET sockfd, bool on);

	static SOCKET connect(SOCKET sockfd, const struct sockaddr_in& addr);

	static void bindOrDie(SOCKET sockfd, const struct sockaddr_in& addr);

	static void listenOrDie(SOCKET sockfd);

	static SOCKET accept(SOCKET sockfd, struct sockaddr_in* addr);

	static int32_t read(SOCKET sockfd, void* buf, int32_t count);
#ifndef WIN32
	static ssize_t readv(SOCKET sockfd, const struct iovec* iov, int iovcnt);
#endif
	static int32_t write(SOCKET sockfd, const void* buf, int32_t count);

	static void close(SOCKET sockfd);

	static void shutdownWrite(SOCKET sockfd);

	static void toIpPort(char* buf, size_t size, const struct sockaddr_in& addr);

	static void toIp(char* buf, size_t size, const struct sockaddr_in& addr);

	static void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

	static int getSocketError(SOCKET sockfd);

	static const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);

	static struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);

	static const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

	static struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);

	static struct sockaddr_in getLocalAddr(SOCKET sockfd);

	static struct sockaddr_in getPeerAddr(SOCKET sockfd);

	static bool isSelfConnect(SOCKET sockfd);
};

END_NS(net)

#endif
