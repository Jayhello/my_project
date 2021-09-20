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
    volatile uint32_t read_num_ = 0;
    volatile bool is_write_ = false;

    std::mutex mtx_;
    std::condition_variable r_ready_, w_ready_;
};

/*
    上面的版本有问题, read_num_ 非线程安全应该为 atomic_int
    同时锁一般都是结合互斥体一起使用上面得显示lock, unlock这样也不安全
    下面看tars中的读写锁
*/
class ShareMutex{
public:
    void lockR();

    void unlockR();

    void lockW();

    void unlockW();
private:
    volatile int read_num_ = 0;
    volatile bool is_write_ = false;
    std::mutex mtx_;
    std::condition_variable cv_read_, cv_write_;
};

template<typename Mutex>
class RLock{
public:
    explicit RLock(Mutex& mtx):mtx_(mtx){
        mtx_.lockR();
        own_ = true;
    }

    ~RLock(){
        if(own_){
            mtx_.unlockR();
        }
    }

    Mutex& mtx_;
    mutable bool own_ = false;
};

template<typename Mutex>
class WLock{
public:
    explicit WLock(Mutex& mtx):mtx_(mtx){
        mtx_.lockW();
        own_ = true;
    }

    ~WLock(){
        if(own_){
            mtx_.unlockW();
        }
    }

    Mutex& mtx_;
    mutable bool own_ = false;
};

typedef WLock<ShareMutex> RW_W_Lock;

typedef RLock<ShareMutex> RW_R_Lock;

} // rw_lock
} // comm
