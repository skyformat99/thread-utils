
#pragma once

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>
#include <chrono>

namespace toolbox
{

template<typename T>
class AsyncQueue
{
public:
    AsyncQueue(unsigned max_queue_size = std::numeric_limits<unsigned>::max()) :
        _max_queue_size(max_queue_size),
        _stop(false)
    { }

    bool getItem(T &item, int timeout_ms = 0)
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);

        if (timeout_ms > 0)
        {
            auto now = std::chrono::system_clock::now();
            if (!_item_posted.wait_until(lock, now + std::chrono::milliseconds(timeout_ms),
                                         [&](){ return !this->_item_queue.empty() || this->_stop; }))
            {
                return false;
            }
        }
        else
        {
            _item_posted.wait(lock,
                              [&](){ return !this->_item_queue.empty() || this->_stop; });
        }

        if (_stop) return false;

        item = std::move(_item_queue.front());

        _item_queue.pop_front();
        _item_consumed.notify_all();

        return true;
    }

    void postItem(T && item)
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);

#if 0
        auto size = _item_queue.size();
        if (size > 2)
        {
            LOGW << "Write queue size: " << size;
        }
#endif

        _item_consumed.wait(lock,
                            [&](){ return this->_item_queue.size() < this->_max_queue_size; });

        //_item_queue.push_back(item);
        _item_queue.emplace_back(std::move(item));

        if (_item_queue.size() > 1)
            _item_posted.notify_all();
        else
            _item_posted.notify_one();
    }

    void flush()
    {
        T t;
        while (!this->empty())
            this->getItem(t);
    }

    void stop()
    {
        _stop = true;
        _item_posted.notify_all();
    }

    int queueSize()
    {
        _queue_mutex.lock();
        unsigned queue_size = _item_queue.size();
        _queue_mutex.unlock();
        return queue_size;
    }

    bool empty()
    {
        return this->queueSize() == 0;
    }

private:
    unsigned                 _max_queue_size;
    bool                     _stop;
    std::mutex               _queue_mutex;
    std::condition_variable  _item_posted;
    std::condition_variable  _item_consumed;
    std::deque<T>            _item_queue;
};

}
