#ifndef __COUNT_DOWN_LATCH_H
#define __COUNT_DOWN_LATCH_H

#include <mutex>
#include <condition_variable> 
#include "common.h"

BEGIN_NS(base)

class BASE_API CountDownLatch
{
public:

    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    mutable std::mutex      mutex_;
    std::condition_variable condition_;
    int                     count_;
};

END_NS(base)

#endif