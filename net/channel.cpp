#include "async_log.h"
#include "channel.h"
#include "event_loop.h"
#include <sstream>
#include <assert.h>

//#include <poll.h>

BEGIN_NS(net)
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = XPOLLIN | XPOLLPRI;
const int Channel::kWriteEvent = XPOLLOUT;

Channel::Channel(EventLoop* loop, SOCKET fd__)
  : loop_(loop),
    fd_(fd__),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{
}

Channel::~Channel()
{
  assert(!eventHandling_);
  assert(!addedToLoop_);
  if (loop_->isInLoopThread())
  {
    assert(!loop_->hasChannel(this));
  }
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Channel::update()
{
  addedToLoop_ = true;
  loop_->updateChannel(this);
}

void Channel::remove()
{
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
  std::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      handleEventWithGuard(receiveTime);
    }
  }
  else
  {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    /*
   XPOLLIN ，读事件
   XPOLLPRI，读事件，但表示紧急数据，例如tcp socket的带外数据
   POLLRDNORM , 读事件，表示有普通数据可读　　　
   POLLRDBAND ,　读事件，表示有优先数据可读　　　　
   XPOLLOUT，写事件
   POLLWRNORM , 写事件，表示有普通数据可写
   POLLWRBAND ,　写事件，表示有优先数据可写　　　   　　　　
   XPOLLRDHUP (since Linux 2.6.17)，Stream socket的一端关闭了连接（注意是stream socket，我们知道还有raw socket,dgram socket），或者是写端关闭了连接，如果要使用这个事件，必须定义_GNU_SOURCE 宏。这个事件可以用来判断链路是否发生异常（当然更通用的方法是使用心跳机制）。要使用这个事件，得这样包含头文件：
   　　#define _GNU_SOURCE
     　　#include <poll.h>
   XPOLLERR，仅用于内核设置传出参数revents，表示设备发生错误
   XPOLLHUP，仅用于内核设置传出参数revents，表示设备被挂起，如果poll监听的fd是socket，表示这个socket并没有在网络上建立连接，比如说只调用了socket()函数，但是没有进行connect。
   XPOLLNVAL，仅用于内核设置传出参数revents，表示非法请求文件描述符fd没有打开
   */
    LOGD(reventsToString().c_str());
    if ((revents_ & XPOLLHUP) && !(revents_ & XPOLLIN))
    {
        if (logHup_)
        {
            LOGW("Channel::handle_event() XPOLLHUP");
        }
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & XPOLLNVAL)
    {
        LOGW("Channel::handle_event() XPOLLNVAL");
    }

    if (revents_ & (XPOLLERR | XPOLLNVAL))
    {
        if (errorCallback_)
            errorCallback_();
    }

    if (revents_ & (XPOLLIN | XPOLLPRI | XPOLLRDHUP))
    {
        //当是侦听socket时，readCallback_指向Acceptor::handleRead
        //当是客户端socket时，调用TcpConnection::handleRead 
        if (readCallback_)
            readCallback_(receiveTime);
    }

    if (revents_ & XPOLLOUT)
    {
        //如果是连接状态服的socket，则writeCallback_指向Connector::handleWrite()
        if (writeCallback_)
            writeCallback_();
    }
    eventHandling_ = false;
}

std::string Channel::reventsToString() const
{
    return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const
{
  return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(SOCKET fd, int ev)
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & XPOLLIN)
    oss << "IN ";
  if (ev & XPOLLPRI)
    oss << "PRI ";
  if (ev & XPOLLOUT)
    oss << "OUT ";
  if (ev & XPOLLHUP)
    oss << "HUP ";
  if (ev & XPOLLRDHUP)
    oss << "RDHUP ";
  if (ev & XPOLLERR)
    oss << "ERR ";
  if (ev & XPOLLNVAL)
    oss << "NVAL ";

  return oss.str();
}

END_NS(net)