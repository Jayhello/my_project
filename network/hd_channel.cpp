//
// Created by wenwen on 2023/5/5.
//

#include "hd_channel.h"
#include "hd_ev.h"

namespace hd{

Channel::Channel(int fd, EventLoop* loop):fd_(fd), loop_(loop), events_(0), revents(0), addedToLoop_(0){
}

Channel::~Channel(){
}

void Channel::enableRead(){

}

void Channel::enableWrite(){

}


} // hd
