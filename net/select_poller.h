#ifndef __NET_SELECT_POLLER_H
#define __NET_SELECT_POLLER_H

#include "poller.h"
#include "platform.h"

BEGIN_NS(net)

class EventLoop;
class Channel;

class SelectPoller : public Poller
{
public:
    SelectPoller(EventLoop* loop);
    virtual ~SelectPoller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;

    virtual bool updateChannel(Channel* channel) override;

    virtual void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels, fd_set& readfds, fd_set& writefds) const;
    bool update(int operation, Channel* channel);

private:
    typedef std::vector<struct epoll_event> EventList;

    EventList       events_;

};

END_NS(net)

#endif

