// File Name: blocking_queue.h
// Author: lei
// Created Time: 
#ifndef _TINY_SRVFRAMEWORK_BLOCKING_QUEUE_H_
#define _TINY_SRVFRAMEWORK_BLOCKING_QUEUE_H_

#include <assert.h>
#include <deque>
#include <utility>  // std::move
#include <mutex>
#include <condition_variable> 

namespace tiny_srv {

template<typename T>
class BlockingQueue {
public:
    BlockingQueue() {}

    void Push(const T &x) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(x);
        m_not_empty_cond.notify_all();
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty()) { m_not_empty_cond.wait(lock); }        
        assert(!m_queue.empty());
        T front(std::move(m_queue.front()));
        m_queue.pop_front();
        return front;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    // noncopyable
    BlockingQueue(const BlockingQueue& queue);
    BlockingQueue& operator=(const BlockingQueue& rhs);

private:
    std::deque<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable_any m_not_empty_cond;
};

}  // end of namespace
#endif
