//
// Created by root on 20-8-9.
//

#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <future>

namespace comm{
namespace thread_pool{

class ThreadPool1{
public:
    ThreadPool1(int num);

    template<typename Fun, typename... Args>
    void emplace(Fun&& fun, Args&&... args);

    ~ThreadPool1();

    typedef std::function<void()> Task;
private:
    std::condition_variable ready_;
    std::mutex mtx_;
    bool stop_;
    std::queue<Task> tasks_;
    std::vector<std::thread> works_;
};

ThreadPool1::ThreadPool1(int num):stop_(false){
    for (int i = 0; i < num; ++i) {
        works_.emplace_back([this](){
            while(true){
                ThreadPool1::Task task;

                {
                    std::unique_lock<std::mutex> lock(this->mtx_);
                    this->ready_.wait(lock, [this](){
                        return this->stop_ or not this->tasks_.empty();
                    });

                    if(this->stop_ and this->tasks_.empty())
                        return;

                    task = std::move(this->tasks_.front());
                    tasks_.pop();
                }

                task();
            }
        });
    }
}

template<typename Fun, typename... Args>
void ThreadPool1::emplace(Fun&& fun, Args&&... args){
    ThreadPool1::Task task = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);

    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        if(stop_) return;

        tasks_.push(std::move(task));
    }

    ready_.notify_one();
}

ThreadPool1::~ThreadPool1() {
    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        stop_ = true;
    }

    ready_.notify_all();

    for(auto& worker : works_){
        worker.join();
    }
}

} // thread
} // comm
