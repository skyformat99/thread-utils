#pragma once

#include "async_queue.h"

#include <thread>
#include <future>
#include <condition_variable>
#include <cassert>

namespace toolbox
{

template<typename J, typename R>
class AsyncWorker
{
    typedef std::pair<std::promise<R>, J> job_type;

public:
    typedef R Result;
    typedef J Job;

    AsyncWorker(int id=0) :
        _id(id),
        _done(false)
        //_job_valid(false)
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

    //bool proceed() const
    //{
        //return _job_valid || _done;
    //}

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
            job_type job;
            if (_job_queue.getItem(job))
            {
#endif
                fmt::printf("GOT JOB: %s\n", std::get<1>(job));
//            _result_valid = false;
//            Result result =
                auto & p = std::get<0>(job);
                p.set_value(this->processJob(std::get<1>(job)));
                //_result_queue->postItem(processJob(_job));
                _job_valid = false;
            }
            if (this->_done) break;

//            _result_valid = true;
//            _job_done.notify_one();
        }
//        printf("Leaving thread...\n");
        this->stopThread();
    }

    std::future<R> postJob(const J & job)
    {
        std::promise<R> p;
        auto fut = p.get_future();
        //auto x =
        _job_queue.postItem(std::make_pair(std::move(p), job));
        return fut;
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

        printf("Stopping worker...\n");
        _done = true;
        _job_queue.stop();
        _thread.join();
    }

    // called from thead before starting job processing loop
    virtual void initThread() { }

    // called from thead before leaving job processing loop
    virtual void stopThread()
    {
        printf("Original stopthread!!?\n");
    }

    // reimplement in subclass to process jobs in thread
    virtual R processJob(const J & job) = 0;

    int id() const { return _id; }

private:
    //AsyncQueue<std::pair<std::promise<R>, J>> _job_queue;
    AsyncQueue<job_type> _job_queue;
    std::thread _thread;

    int _id;
    bool _done, _job_valid;//, _result_valid;

    //std::pair<std::future<R>, J> _job;
};

} // namespace toolbox

