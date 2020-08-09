//
// Created by root on 20-8-9.
//

#pragma once

#include <mutex>
#include <condition_variable>

namespace comm{
namespace rw_lock{

/*
 * an not good method from github
 */
class RWLock{
public:
    RWLock();

    void lockR();
    void unlockR();

    void lockW();
    void unlockW();

private:
    std::mutex mtx_;
    std::condition_variable r_ready_, w_ready_;

    volatile uint32_t read_num_;
    volatile bool is_write_;
};

RWLock::RWLock():read_num_(0), is_write_(false){}

void RWLock::lockR(){
    std::unique_lock<std::mutex> lock(mtx_);

    r_ready_.wait(lock, [&]{
       return !is_write_;
    });

    ++read_num_;
}

void RWLock::unlockR(){
    std::unique_lock<std::mutex> lock(mtx_);

    --read_num_;
    if(0 == read_num_){
        w_ready_.notify_one();
    }
}

void RWLock::lockW(){
    std::unique_lock<std::mutex> lock(mtx_);

    w_ready_.wait(lock, [&]{
        return !is_write_ and 0 == read_num_;
    });

    is_write_ = true;
}

void RWLock::unlockW(){
    std::unique_lock<std::mutex> lock(mtx_);
    is_write_ = false;

    w_ready_.notify_one();
    r_ready_.notify_all();
}

} // rw_lock
} // comm