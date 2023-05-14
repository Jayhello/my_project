//
// Created by wenwen on 2023/2/3.
// hd_xxx代表的是handy类似的lib

#pragma once
#include "common/noncopyable.h"
#include <functional>
#include <poll.h>

namespace hd{

class EventLoop;

const int kMaxEvents = 2000;
const int kReadEvent = POLLIN;
const int kWriteEvent = POLLOUT;

class Channel{
public:
    using EventCallback = std::function<void()>;

    Channel(int fd, EventLoop* loop);

    ~Channel();

    int fd()const{return fd_;}

    void enableRead();

    void enableWrite();

    void handleRead(){readCallback_();}

    void handleWrite(){writeCallback_();}

private:
    int           fd_;
    EventLoop*    loop_;
    int           events_;
    int           revents_;
    bool          addedToLoop_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
};

} // hd
