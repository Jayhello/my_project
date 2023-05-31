//
// Created by wenwen on 2023/2/3.
// hd_xxx代表的是handy类似的lib

#pragma once
#include "common/noncopyable.h"
#include <functional>
#include <poll.h>

namespace hd{

class EventLoop;

const int kReadEvent = POLLIN;
const int kWriteEvent = POLLOUT;

class Channel{
public:
    using EventCallback = std::function<void()>;

    Channel(int fd, EventLoop* loop);

    ~Channel();

    int fd()const{return fd_;}

    int events()const{return events_;}

    int revents()const{return revents_;}

    void setRevents(int revents){revents_ = revents;}

    void enableRead();

    void disableRead();

    void enableWrite();

    void disableWrite();

    void setReadCallback(const EventCallback& cb){readCallback_ = cb;}

    void setWriteCallback(const EventCallback& cb){writeCallback_ = cb;}

    void handleRead(){readCallback_();}

    void handleWrite(){writeCallback_();}

private:
    void updateEvent();

private:
    int           fd_;
    EventLoop*    loop_;
    int           events_;
    int           revents_;
    bool          addedToLoop_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
};

using ChannelPtr = Channel*;

} // hd
