//
// Created by wenwen on 2023/2/3.
//

#include "hd_ev.h"

namespace hd{

int EpollTimer::init(){
    efd_ = epoll_create(1);
    if(efd_ < 0) return -1;

    events_ = new epoll_event[MAX_EVENTS];
    bzero(events_, sizeof(*events_) * MAX_EVENTS);

    return 0;
}

int EpollTimer::loop(){

    return 0;
}

int EpollTimer::loopOnce(){
    int num = epoll_wait(efd_, events_, MAX_EVENTS, timeOutMs_);
    return_ret_if(num < 0, num, "epoll_wait_fail");

    for(int i = 0; i < num; ++i){}

    long lNowMs = NOW_MS;
    auto it = idTask_.begin();
    while(it != idTask_.end()){
        if(it->first.lTimeMs > lNowMs){
            break;
        }

        it->second.doTask();

        const auto & tm = it->first;
        const auto & task = it->second;

        if(task.isLoopTask()){
            TimerId tNewId = {lNowMs + task.getIntervalMs()};
            TimerTask timerTask = TimerTask(task.getCaller(), );
            auto item = std::make_pair<TimerId, TimerTask>(, );
            idTask_.insert();
        }else{
            idTask_.erase(it->first);
        }
    }

    return 0;
}


} // hd
