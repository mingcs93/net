#ifndef __NET_EVENTLOOPTHREADPOOL_H
#define __NET_EVENTLOOPTHREADPOOL_H
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include "common.h"

BEGIN_NS(net)

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThreadPool();
	~EventLoopThreadPool();

	void init(EventLoop* baseLoop, int numThreads);
	void start(const ThreadInitCallback& cb = ThreadInitCallback());

	void stop();

	// valid after calling start()
	/// round-robin
	EventLoop* getNextLoop();

	/// with the same hash code, it will always return the same EventLoop
	EventLoop* getLoopForHash(size_t hashCode);

	std::vector<EventLoop*> getAllLoops();

	bool started() const
	{
		return started_;
	}

	const std::string& name() const
	{
		return name_;
	}

	const std::string info() const;

private:

	EventLoop* baseLoop_;
	std::string                                     name_;
	bool                                            started_;
	int                                             numThreads_;
	int                                             next_;
	std::vector<std::unique_ptr<EventLoopThread> >  threads_;
	std::vector<EventLoop*>                         loops_;
};

END_NS(net)

#endif