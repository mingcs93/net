#ifndef __NET_EVENTLOOPTHREAD_H
#define __NET_EVENTLOOPTHREAD_H

#include <mutex>
#include <condition_variable> 
#include <thread>
#include <string>
#include <functional>
#include "common.h"

BEGIN_NS(net)

class EventLoop;

class EventLoopThread
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = "");
	~EventLoopThread();
	EventLoop* startLoop();
	void stopLoop();

private:
	void threadFunc();

	EventLoop* loop_;
	bool                         exiting_;
	std::unique_ptr<std::thread> thread_;
	std::mutex                   mutex_;
	std::condition_variable      cond_;
	ThreadInitCallback           callback_;
};
	
END_NS(net)

#endif