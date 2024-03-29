//
// Created by Administrator on 2022/10/10.
//

#include "thread_pool.h"


namespace comm{
namespace thread_pool{

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

void Thread::start(){
    {
        // try_lock就可以很好的防止并发开启线程
        std::unique_lock<std::mutex> lk(mtx_, std::try_to_lock);
        if(not lk.owns_lock() or running_){
            throw std::logic_error("thread has start");
        }

        running_ = true;
    }

    th_.reset(new std::thread([this](){
        this->run();
        running_ = false;
    }));
    //    running_ = false;
}

} // thread_pool
} // comm