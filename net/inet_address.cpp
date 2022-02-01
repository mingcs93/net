#include "inet_address.h"
#include "async_log.h"
#include "net_endian.h"
#include "socketsOps.h"
#include <cstring>

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

//static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in), "sizeof(InetAddress) == sizeof(struct sockaddr_in)");

BEGIN_NS(net)

InetAddress::InetAddress(uint16_t port, bool loopbackOnly/* = false*/)
{
	memset(&addr_, 0, sizeof addr_);
	addr_.sin_family = AF_INET;
	in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
	addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
	addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    memset(&addr_, 0, sizeof addr_);
	SocketsOps::fromIpPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toIpPort() const
{
	char buf[32];
	SocketsOps::toIpPort(buf, sizeof buf, addr_);
	return buf;
}

std::string InetAddress::toIp() const
{
	char buf[32];
	SocketsOps::toIp(buf, sizeof buf, addr_);
	return buf;
}

uint16_t InetAddress::toPort() const
{
	return sockets::networkToHost16(addr_.sin_port);
}

static thread_local char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(const std::string& hostname, InetAddress* out)
{
	//assert(out != nullptr);
	struct hostent hent;
	struct hostent* he = nullptr;
	int herrno = 0;
	memset(&hent, 0, sizeof(hent));

#ifndef WIN32
	int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
	if (ret == 0 && he != nullptr)
	{
		//assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
		out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		return true;
	}

	if (ret)
	{
		LOGSYSE("InetAddress::resolve");
	}

#endif
    //TODO: Windows上重新实现一下
	return false;
}

END_NS(net)
