//
// Created by wenwen on 2023/5/5.
//

#include "hd_channel.h"
#include "hd_ev.h"

namespace hd{

Channel::Channel(int fd, EventLoop* loop):fd_(fd), loop_(loop), events_(0), revents(0), addedToLoop_(0){
    loop_->getPoller()->addChannel(this);
}

Channel::~Channel(){
}

void Channel::updateEvent(){
    loop_->getPoller()->updateChannel(this);
}

void Channel::enableRead(){
    events_ |= kReadEvent;
    updateEvent();
}

void Channel::disableRead(){
    events_ &= ~kReadEvent;
    updateEvent();
}

void Channel::enableWrite(){
    events_ |= kWriteEvent;
    updateEvent();
}

void Channel::disableWrite(){
    events_ &= ~kWriteEvent;
    updateEvent();
}


} // hd
