#ifndef __NET_ACCEPTOR_H
#define __NET_ACCEPTOR_H

#include <functional>

#include "channel.h"
#include "socket.h"

BEGIN_NS(net)

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor
{
public:
	typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{
		newConnectionCallback_ = cb;
	}

	void listen();

	bool listening() const { return listening_; }

	// Deprecated, use the correct spelling one above.
	// Leave the wrong spelling here in case one needs to grep it for error messages.
	// bool listenning() const { return listening(); }

private:
	void handleRead();

	EventLoop* loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;
	bool listening_;
#ifndef WIN32
	int                   idleFd_;
#endif
};

END_NS(net)

#endif  // __NET_ACCEPTOR_H
