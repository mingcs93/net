#ifndef __NET_TIMERID_H
#define __NET_TIMERID_H

#include "common.h"

BEGIN_NS(net)

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId
{
 public:
  TimerId()
    : timer_(nullptr),
      sequence_(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
  {
  }

  // default copy-ctor, dtor and assignment are okay

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};

END_NS(net)

#endif  // __NET_TIMERID_H
