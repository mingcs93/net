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
   XPOLLIN �����¼�
   XPOLLPRI�����¼�������ʾ�������ݣ�����tcp socket�Ĵ�������
   POLLRDNORM , ���¼�����ʾ����ͨ���ݿɶ�������
   POLLRDBAND ,�����¼�����ʾ���������ݿɶ���������
   XPOLLOUT��д�¼�
   POLLWRNORM , д�¼�����ʾ����ͨ���ݿ�д
   POLLWRBAND ,��д�¼�����ʾ���������ݿ�д������   ��������
   XPOLLRDHUP (since Linux 2.6.17)��Stream socket��һ�˹ر������ӣ�ע����stream socket������֪������raw socket,dgram socket����������д�˹ر������ӣ����Ҫʹ������¼������붨��_GNU_SOURCE �ꡣ����¼����������ж���·�Ƿ����쳣����Ȼ��ͨ�õķ�����ʹ���������ƣ���Ҫʹ������¼�������������ͷ�ļ���
   ����#define _GNU_SOURCE
     ����#include <poll.h>
   XPOLLERR���������ں����ô�������revents����ʾ�豸��������
   XPOLLHUP���������ں����ô�������revents����ʾ�豸���������poll������fd��socket����ʾ���socket��û���������Ͻ������ӣ�����˵ֻ������socket()����������û�н���connect��
   XPOLLNVAL���������ں����ô�������revents����ʾ�Ƿ������ļ�������fdû�д�
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
        //��������socketʱ��readCallback_ָ��Acceptor::handleRead
        //���ǿͻ���socketʱ������TcpConnection::handleRead 
        if (readCallback_)
            readCallback_(receiveTime);
    }

    if (revents_ & XPOLLOUT)
    {
        //���������״̬����socket����writeCallback_ָ��Connector::handleWrite()
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