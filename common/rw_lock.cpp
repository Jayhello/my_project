//
// Created by wenwen on 2021/9/19.
//
#include "rw_lock.h"

namespace comm{
namespace rw_lock{

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

void ShareMutex::lockR(){
    std::unique_lock<std::mutex> lk(mtx_);
    cv_read_.wait(lk, [&](){
        return !is_write_;
    });

    ++read_num_;
}

void ShareMutex::unlockR(){
    std::unique_lock<std::mutex> lk(mtx_);
    --read_num_;
    if(0 <= read_num_){
        cv_write_.notify_one();
    }
}

void ShareMutex::lockW(){
    std::unique_lock<std::mutex> lk(mtx_);
    cv_read_.wait(lk, [&](){
        return !is_write_ and read_num_ <= 0;
    });

    is_write_ = true;
}

void ShareMutex::unlockW(){
    std::unique_lock<std::mutex> lk(mtx_);
    is_write_ = false;

    cv_write_.notify_one();
    cv_read_.notify_all();
}

} // rw_lock
} // comm