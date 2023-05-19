//
// Created by wenwen on 2023/5/5.
// hd_xxx代表的是handy类似的lib

#pragma once
#include "common/noncopyable.h"
#include "hd_channel.h"
#include "hd_endpoint.h"

namespace hd{

class EventLoop;

using AcceptCallback = std::function<void()>;

class Acceptor{
public:
    Acceptor(EventLoop*, int port);

    void setAcceptCallback(AcceptCallback&& cb){
        acceptCallback_ = std::move(cb);
    }

private:
    void handleAccept();

private:
    EventLoop* loop_;
    ChannelPtr acceptChannel_;
    EndPoint   ePoint;

    AcceptCallback acceptCallback_;
};

} // hd
