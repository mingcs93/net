#include "event_loop.h"
#include "async_log.h"

#include "channel.h"
#include "socketsOps.h"
#ifdef WIN32
#include "select_poller.h"
#else
#include "epoll_poller.h"
#endif

#include <sstream>
#include <algorithm>
#include <assert.h>
#include <string.h>

BEGIN_NS(net)

const int kPollTimeMs = 100;

thread_local  EventLoop* t_loopInThisThread = 0;

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , eventHandling_(false)
    , callingPendingFunctors_(false)
    , iteration_(0)
    , threadId_(std::this_thread::get_id())
    //, timerQueue_(new TimerQueue(this))
    , currentActiveChannel_(nullptr)
{
    createWakeupfd();

#ifdef WIN32
    wakeupChannel_.reset(new Channel(this, wakeupFdRecv_));
    poller_.reset(new SelectPoller(this));

#else
    wakeupChannel_.reset(new Channel(this, wakeupFd_));
    poller_.reset(new EPollPoller(this));
    /*wakeupChannel_.reset(new Channel(this, wakeupFd_));
    poller_.reset(new SelectPoller(this));*/
#endif

    if (t_loopInThisThread)
    {
        LOGF("Another EventLoop  exists in this thread ");
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    assertInLoopThread();
    LOGD("EventLoop 0x%x destructs.", this);

    wakeupChannel_->disableAll();
    wakeupChannel_->remove();

#ifdef WIN32
    SocketsOps::close(wakeupFdSend_);
    SocketsOps::close(wakeupFdRecv_);
    SocketsOps::close(wakeupFdListen_);
#else
    SocketsOps::close(wakeupFd_);
#endif
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
    LOGD("EventLoop 0x%x  start looping", this);

    while (!quit_)
    {
        //timerQueue_->doTimer();

        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        //if (Logger::logLevel() <= Logger::TRACE)
        //{
        printActiveChannels();
        //}
        ++iteration_;
        // TODO sort channel by priority
        eventHandling_ = true;
        for (const auto& it : activeChannels_)
        {
            currentActiveChannel_ = it;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();

        /*if (frameFunctor_)
        {
            frameFunctor_();
        }*/
    }

    LOGD("EventLoop 0x%0x stop looping", this);
    looping_ = false;


    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    LOGI("Exiting loop, EventLoop object: 0x%x , threadID: %s", this, stid.c_str());
}

void EventLoop::quit()
{
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    //return timerQueue_->addTimer(std::move(cb), time, 0.0);
    return TimerId{};
}

TimerId EventLoop::runAfter(int64_t delay, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(int64_t interval, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    //return timerQueue_->addTimer(std::move(cb), time, interval);
    return TimerId{};
}

void EventLoop::cancel(TimerId timerId)
{
    //return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
            std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

bool EventLoop::createWakeupfd()
{
#ifdef WIN32
    wakeupFdListen_ = SocketsOps::createOrDie();
    wakeupFdSend_ = SocketsOps::createOrDie();

    //Windows上需要创建一对socket  
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //将port设为0，然后进行bind，再接着通过getsockname来获取port，这可以满足获取随机端口的情况。
    bindaddr.sin_port = 0;
    SocketsOps::setReuseAddr(wakeupFdListen_, true);
    SocketsOps::bindOrDie(wakeupFdListen_, bindaddr);
    SocketsOps::listenOrDie(wakeupFdListen_);

    struct sockaddr_in serveraddr;
    int serveraddrlen = sizeof(serveraddr);
    if (getsockname(wakeupFdListen_, (sockaddr*)&serveraddr, &serveraddrlen) < 0)
    {
        //让程序挂掉
        LOGF("Unable to bind address info, EventLoop: 0x%x", this);
        return false;
    }

    int useport = ntohs(serveraddr.sin_port);
    LOGD("wakeup fd use port: %d", useport);
  
    if (::connect(wakeupFdSend_, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        //让程序挂掉
        LOGF("Unable to connect to wakeup peer, EventLoop: 0x%x", this);
        return false;
    }

    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    wakeupFdRecv_ = ::accept(wakeupFdListen_, (struct sockaddr*)&clientaddr, &clientaddrlen);
    if (wakeupFdRecv_ < 0)
    {
        //让程序挂掉
        LOGF("Unable to accept wakeup peer, EventLoop: 0x%x", this);
        return false;
    }

    SocketsOps::setNonBlockAndCloseOnExec(wakeupFdSend_);
    SocketsOps::setNonBlockAndCloseOnExec(wakeupFdRecv_);

#else
    //Linux上一个eventfd就够了，可以实现读写
    wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeupFd_ < 0)
    {
        //让程序挂掉
        LOGF("Unable to create wakeup eventfd, EventLoop: 0x%x", this);
        return false;
    }

#endif

    return true;
}

void EventLoop::abortNotInLoopThread()
{
    std::stringstream ss;
    ss << "threadid_ = " << threadId_ << " this_thread::get_id() = " << std::this_thread::get_id();
    LOGF("EventLoop::abortNotInLoopThread - EventLoop %s", ss.str().c_str());
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
#ifdef WIN32
    int32_t n = SocketsOps::write(wakeupFdSend_, &one, sizeof(one));
#else
    int32_t n = SocketsOps::write(wakeupFd_, &one, sizeof(one));
#endif


    if (n != sizeof one)
    {
#ifdef WIN32
        DWORD error = WSAGetLastError();
        LOGSYSE("EventLoop::wakeup() writes %d  bytes instead of 8, fd: %d, error: %d", n, wakeupFdSend_, (int32_t)error);
#else
        int error = errno;
        LOGSYSE("EventLoop::wakeup() writes %d  bytes instead of 8, fd: %d, error: %d, errorinfo: %s", n, wakeupFd_, error, strerror(error));
#endif


        return;
    }

    return;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
#ifdef WIN32
    int32_t n = SocketsOps::read(wakeupFdRecv_, &one, sizeof(one));
#else
    int32_t n = SocketsOps::read(wakeupFd_, &one, sizeof(one));
#endif

    if (n != sizeof one)
    {
#ifdef WIN32
        DWORD error = WSAGetLastError();
        LOGSYSE("EventLoop::wakeup() read %d  bytes instead of 8, fd: %d, error: %d", n, wakeupFdRecv_, (int32_t)error);
#else
        int error = errno;
        LOGSYSE("EventLoop::wakeup() read %d  bytes instead of 8, fd: %d, error: %d, errorinfo: %s", n, wakeupFd_, error, strerror(error));
#endif
        return;
    }

    return;
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
    for (const Channel* channel : activeChannels_)
    {
        LOGD("{%s}", channel->reventsToString().c_str());
    }
}

END_NS(net)