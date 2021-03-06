#include "event_loop_thread_pool.h"
#include <assert.h>
#include <sstream>
#include <string>
#include "event_loop.h"
#include "event_loop_thread.h"
#include "callbacks.h"

BEGIN_NS(net)

EventLoopThreadPool::EventLoopThreadPool()
    : baseLoop_(nullptr),
    started_(false),
    numThreads_(0),
    next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::init(EventLoop* baseLoop, int numThreads)
{
    numThreads_ = numThreads;
    baseLoop_ = baseLoop;
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(baseLoop_);
    if (baseLoop_ == nullptr)
        return;

    assert(!started_);
    if (started_)
        return;

    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[128];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);

        std::unique_ptr<EventLoopThread> t(std::make_unique<EventLoopThread>(cb, buf));
        //EventLoopThread* t = new EventLoopThread(cb, buf);		
        loops_.push_back(t->startLoop());
        threads_.push_back(std::move(t));
    }
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

void EventLoopThreadPool::stop()
{
    for (auto& iter : threads_)
    {
        iter->stopLoop();
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (!started_)
        return nullptr;

    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    baseLoop_->assertInLoopThread();
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}

const std::string EventLoopThreadPool::info() const
{
    std::stringstream ss;
    ss << "print threads id info " << std::endl;
    for (size_t i = 0; i < loops_.size(); i++)
    {
        ss << i << ": id = " << loops_[i]->getThreadID() << std::endl;
    }
    return ss.str();
}

END_NS(net)