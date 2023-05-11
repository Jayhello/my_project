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
#include <atomic>
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

class PollerBase;
using PollerPtr = PollerBase*;

class Channel;
using ChannelPtr = Channel*;

class EventLoop{
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void updateChannel(ChannelPtr);

    void stop(){stop_ = true;}

private:
    PollerPtr   poll_;
    bool        stop_;
};


struct TimerTask{
    enum eTaskType{
        ONCE = 0,
        LOOP = 1,
    };

    using Caller = std::function<void(void)>;

    TimerTask(Caller cl, int t = ONCE, long lIntervalMs = 0):call_(cl), type_(t), lIntervalMs_(lIntervalMs){
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
    bool operator < (const TimerId& t1)const{
        return lTimeMs < t1.lTimeMs;
    }
};

struct EpollTimer{
    const static int MAX_EVENTS = 100;

    int init();

    int loop();

    int loopOnce();

    bool hasStop()const{return bStop_;}

    void stop(){
        bStop_ = true;
    }

    int genAutoId(){
        return ++iAutoId_;
    }

    template<typename Fun, typename... Args>
    void runAt(long lTsMs, Fun&& fun, Args&&... args){
        auto f = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);
        TimerTask task(f);
        addTask(lTsMs, task);
    }

    template<typename Fun, typename... Args>
    void runAfter(long lTsMs, Fun&& fun, Args&&... args){
        auto f = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);
        TimerTask task(f);
        addTask(lTsMs + NOW_MS, task);
    }

    template<typename Fun, typename... Args>
    void runEvery(long lInterval, Fun&& fun, Args&&... args){
        auto f = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);
        TimerTask task(f, TimerTask::LOOP, lInterval);
        addTask(lInterval + NOW_MS, task);
    }

    void addTask(long lRunMs, const TimerTask& task){
        long lId = genAutoId();
        TimerId timerId{lId, lRunMs};
        idTask_.insert({timerId, task});
    }

private:
    int  efd_                       = -1;
    struct epoll_event*     events_ = nullptr;
    std::multimap<TimerId, TimerTask> idTask_;
    int  timeOutMs_                 = 1000;
    bool bStop_                     = false;
    std::atomic_int iAutoId_;
};


} // hd
