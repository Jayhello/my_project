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
#include "common/commmon.h"
#include <list>

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

template<typename T>
class SafeQueue: private comm::util::noncopyable{
public:
    static const int WAIT_INFINITE = std::numeric_limits<int>::max();

    bool push(const T& val);

    T pop_wait(int waitMs = WAIT_INFINITE);

    bool pop_wait(T* p_v, int waitMs = WAIT_INFINITE);

    size_t size();

    void exit();

    bool exited()const {return exit_;}

protected:
    void wait_ready(std::unique_lock<std::mutex> &lk, int waitMs);

private:
    std::list<T> queue_;
    std::mutex mtx_;
    std::condition_variable ready_;
    std::atomic_bool exit_;
};

typedef std::function<void()> Task;

class ThreadPool : private comm::util::noncopyable{
public:
    ThreadPool(int num);
    ~ThreadPool();

    void addTask(Task&& task);

public:
    std::vector<std::thread> threads_;
    SafeQueue<Task> tasks_;
};

ThreadPool::ThreadPool(int num) {
    threads_.reserve(num);

    for (int i = 0; i < num; ++i) {
        threads_.emplace_back([this](){
            while(not tasks_.exited()){
                Task task;

                if(tasks_.pop_wait(&task)){
                    task();
                }
            }
        });
    }
}

void ThreadPool::addTask(Task&& task){
    tasks_.push(std::move(task));
}

ThreadPool::~ThreadPool(){
    tasks_.exit();

    for(auto& th:threads_){
        th.join();
    }
}

template<typename T>
bool SafeQueue<T>::push(const T& val){
    std::unique_lock<std::mutex> lock(mtx_);

    if(exit_){
        return false;
    }

    queue_.push_back(val);
    ready_.notify_one();
}

template<typename T>
void SafeQueue<T>::wait_ready(std::unique_lock<std::mutex> &lk, int waitMs){
    if( WAIT_INFINITE == waitMs){
        ready_.wait(lk, [this](){
            return not this->queue_.empty() or exit_;
        });
    }else{
        auto tp = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitMs);
        while (ready_.wait_until(lk, tp) != std::cv_status::timeout && queue_.empty() && !exit_) {
        }
    }
}

template<typename T>
T SafeQueue<T>::pop_wait(int waitMs){
    std::unique_lock<std::mutex> lock(mtx_);

    wait_ready(lock,waitMs);

    if(queue_.empty()){
        return T();
    }

    T ret = queue_.back();
    queue_.pop_back();

    return ret;
}

template<typename T>
bool SafeQueue<T>::pop_wait(T* p_v, int waitMs){
    std::unique_lock<std::mutex> lock(mtx_);

    wait_ready(lock,waitMs);

    if(queue_.empty()){
        return false;
    }

    // p_v = new T(queue_.back()); // below is better
    *p_v = std::move(queue_.back());
    queue_.pop_back();

    return true;
}

template<typename T>
size_t SafeQueue<T>::size(){
    std::unique_lock<std::mutex> lock(mtx_);

    return queue_.size();
}

template<typename T>
void SafeQueue<T>::exit(){
    exit_ = true;

    std::unique_lock<std::mutex> lock(mtx_);
    ready_.notify_all();
}

} // thread
} // comm
