#include "event_loop_thread.h"
#include <functional>
#include "event_loop.h"

BEGIN_NS(net)

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
								 const std::string& name/* = ""*/)
								 : loop_(nullptr),
								 exiting_(false),
								 callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	if (loop_ != nullptr) // not 100% race-free, eg. threadFunc could be running callback_.
	{
		// still a tiny chance to call destructed object, if threadFunc exits just now.
		// but when EventLoopThread destructs, usually programming is exiting anyway.
		loop_->quit();
		thread_->join();
	}
}

EventLoop* EventLoopThread::startLoop()
{
	//assert(!thread_.started());
	//thread_.start();

	thread_.reset(new std::thread(std::bind(&EventLoopThread::threadFunc, this)));

	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (loop_ == nullptr)
		{
			cond_.wait(lock);
		}
	}

	return loop_;
}

void EventLoopThread::stopLoop()
{
    if (loop_ != nullptr)
        loop_->quit();

    thread_->join();
}

void EventLoopThread::threadFunc()
{
	EventLoop loop;

	if (callback_)
	{
		callback_(&loop);
	}

	{
		//一个一个的线程创建
        std::unique_lock<std::mutex> lock(mutex_);
		loop_ = &loop;
		cond_.notify_all();
	}

	loop.loop();
	//assert(exiting_);
	loop_ = nullptr;
}

END_NS(net)