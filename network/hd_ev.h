//
// Created by wenwen on 2023/2/3.
// hd_xxx代表的是handy类似的lib

#pragma once
#include <fcntl.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>//epoll
#include <map>
#include <functional>
#include <iostream>
#include "common/logging.h"
#include "common/commmon.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;
using namespace comm;
using namespace comm::log;
using log_v1::ScopeLog;

namespace hd{

struct TimerTask{
    enum eTaskType{
        ONCE = 0,
        LOOP = 1,
    };

    using Caller = std::function<void(void)>;

    TimerTask(Caller cl, int t, long lIntervalMs):call_(cl), type_(t), lIntervalMs_(lIntervalMs){
    }

    long getIntervalMs()const{
        return lIntervalMs_;
    }

    bool isLoopTask()const{
        return type_ == LOOP;
    }

    int getType()const{return type_;}

    const Caller& getCaller()const{
        return call_;
    }

    void doTask()const{
        call_();
    }

    Caller call_;
    int    type_ = ONCE;
    long   lIntervalMs_ = 0;
};

struct TimerId{
    long lId;       // id(同一个Timer里面自增唯一)
    long lTimeMs;   // 毫秒时间戳
};

struct EpollTimer{
    const static int MAX_EVENTS = 100;

    int init();

    int loop();

    int loopOnce();

    template<typename Fun, typename... Args>
    void runAt(long lTs, Fun&& fun, Args&&... args){

    }

    template<typename Fun, typename... Args>
    void runEvery(long lInterval, Fun&& fun, Args&&... args){

    }

    template<typename Fun, typename... Args>
    void runAfter(long lTs, Fun&& fun, Args&&... args){

    }

    int  efd_                       = -1;
    struct epoll_event*     events_ = nullptr;
    std::multimap<TimerId, TimerTask> idTask_;
    int  timeOutMs_                 = 1000;
};


} // hd
