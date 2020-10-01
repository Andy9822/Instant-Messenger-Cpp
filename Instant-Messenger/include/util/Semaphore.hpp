#pragma once
#ifndef __semaphore_h__
#define __semaphore_h__

#include <mutex>
#include <condition_variable>

using std::mutex;
using std::condition_variable;

class Semaphore {

public:
    Semaphore(int init);
    void wait();
    void post();

private:
    int m_value; // semaphore value
    mutex m_mux;
    condition_variable m_waitcond;
};

#endif