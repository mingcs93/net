#ifndef __NET_POLLER_EPOLLPOLLER_H
#define __NET_POLLER_EPOLLPOLLER_H

#ifndef WIN32

#include "poller.h"
#include <vector>

struct epoll_event;

BEGIN_NS(net)

///
/// IO Multiplexing with epoll(4).
///
class EPollPoller : public Poller
{
public:
	EPollPoller(EventLoop* loop);

	virtual ~EPollPoller();

	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

	virtual bool updateChannel(Channel* channel) override;

	virtual void removeChannel(Channel* channel) override;

private:
	static const int kInitEventListSize = 16;

	void fillActiveChannels(int numEvents,
		ChannelList* activeChannels) const;

	bool update(int operation, Channel* channel);

	typedef std::vector<struct epoll_event> EventList;

	int epollfd_;
	EventList events_;
};

END_NS(net)

#endif //WIN32

#endif