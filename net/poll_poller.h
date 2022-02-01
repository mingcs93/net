#ifndef __NET_POLLER_POLLPOLLER_H
#define __NET_POLLER_POLLPOLLER_H

#ifndef WIN32

#include "poller.h"
#include <vector>

struct pollfd;

BEGIN_NS(net)

///
/// IO Multiplexing with poll(2).
///
class PollPoller : public Poller
{
public:
	PollPoller(EventLoop* loop);

	virtual ~PollPoller();

	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

	virtual bool updateChannel(Channel* channel) override;

	virtual void removeChannel(Channel* channel) override;

private:
	void fillActiveChannels(int numEvents,
		ChannelList* activeChannels) const;

	typedef std::vector<struct pollfd> PollFdList;
	PollFdList pollfds_;
};

END_NS(net)

#endif //WIN32

#endif 
