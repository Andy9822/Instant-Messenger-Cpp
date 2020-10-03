#include <mutex>
#include <condition_variable>
#include "../../include/util/Semaphore.hpp"


using std::unique_lock;
using std::mutex;

Semaphore::Semaphore(int init) {
    this->m_value = init;
}

void Semaphore::wait() {
    unique_lock<mutex> lck(m_mux);
    // make us wait
    if (--m_value < 0) {
        m_waitcond.wait(lck);
    }
}

void Semaphore::post() {
    unique_lock<mutex> lck(m_mux);
    if (++m_value <= 0) m_waitcond.notify_one();
}