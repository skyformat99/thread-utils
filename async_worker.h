#pragma once

#include "async_queue.h"

#include <thread>
#include <condition_variable>
#include <cassert>

namespace toolbox
{

template<typename J, typename R>
class AsyncWorker
{
public:
    typedef R Result;
    typedef J Job;

    AsyncWorker(AsyncQueue<R>* result_queue, int id) :
        _result_queue(result_queue),
        _id(id),
        _done(false),
        _job_valid(false)
    {}

    virtual ~AsyncWorker()
    {
        if (_thread.joinable())
            stop();
    }

    void start()
    {
        _thread = std::thread(&AsyncWorker::run, this);
    }

    bool proceed() const
    {
        return _job_valid || _done;
    }

    void run()
    {
        initThread();
        while(!_done)
        {
#if 0
            std::unique_lock<std::mutex> lock(_mutex);

            _job_posted.wait(lock,
                             [&](){ return this->proceed(); });

#else
            if (_job_queue.getItem(_job))
            {
#endif
//            _result_valid = false;
//            Result result =
                _result_queue->postItem(processJob(_job));
                _job_valid = false;
            }
            if (this->_done) break;

//            _result_valid = true;
//            _job_done.notify_one();
        }
//        printf("Leaving thread...\n");
        this->stopThread();

    }

    void postJob(const J & job)
    {
        _job_queue.postItem(job);
    }


#if 0
    R getResult()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_result_valid)
        {
            _job_done.wait(lock,
                           [&](){ return this->_result_valid; });
        }
        return _result;
    }
#endif
    void stop()
    {
        _done = true;
        _job_queue.stop();
        _thread.join();
    }

    // called from thead before starting job processing loop
    virtual void initThread() { }

    // called from thead before leaving job processing loop
    virtual void stopThread() = 0;
//    {
 //       printf("Original stopthread!!?\n");
//    }

    // reimplement in subclass to process jobs in thread
    virtual R processJob(const J & job) = 0;

    int id() const { return _id; }

private:
    AsyncQueue<J> _job_queue;
    AsyncQueue<R>* _result_queue;
    std::thread _thread;
//    std::mutex _mutex;
//    std::condition_variable _job_posted;

    int _id;
    bool _done, _job_valid;//, _result_valid;

    J _job;
//    R _result;


//    std::condition_variable _job_done;
};
