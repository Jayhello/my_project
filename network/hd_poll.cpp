//
// Created by wenwen on 2023/2/3.
//

#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>
#include "hd_poll.h"
#include "common/logging.h"
#include "hd_channel.h"

namespace hd{

static const int kMaxEvents = 2000;

struct PollerEpoll : public PollerBase{
    PollerEpoll();

    virtual ~PollerEpoll();

    virtual void loopOnce(int iWaitMs, ChannelPtrList& vActiveList);

    virtual void addChannel(ChannelPtr);
    virtual void removeChannel(ChannelPtr);
    virtual void updateChannel(ChannelPtr);

private:
    int                 epfd_   = -1;
    struct epoll_event  events_[kMaxEvents];
};

using namespace::comm::log;

PollerEpoll::PollerEpoll(){
    epfd_ = epoll_create(1);
    fatalif(epfd_ < 0, "epoll_create fail %d %s", errno, strerror(errno));

    info("create_epoll: %d", epfd_);
}

PollerEpoll::~PollerEpoll(){
    if(epfd_ > 0){
        close(epfd_);
    }
    info("epoll fd: %d destroy", epfd_);
}

void PollerEpoll::loopOnce(int iWaitMs, ChannelPtrList& vActiveList){
    int num = epoll_wait(epfd_, events_, kMaxEvents, iWaitMs);
    fatalif(num < 0, "epoll_wait fail %d %s", errno, strerror(errno));

    for (int i = 0; i < num; ++i) {

    }
}

void PollerEpoll::addChannel(ChannelPtr ptr){
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));
    epev.events = ptr->events();
    epev.data.fd = ptr->fd();
    epev.data.ptr = ptr;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, ptr->fd(), &epev);
    trace("add channel fd: %d, epfd: %d, add_ret", ptr->fd(), epfd_, ret);
    fatalif(ret < 0, "add channel fd: %d, epfd: %d, epctl_fail: %d, %s", ptr->fd(), epfd_, errno, strerror(errno));
}

void PollerEpoll::removeChannel(ChannelPtr ptr){
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));
    epev.events = ptr->events();
    epev.data.fd = ptr->fd();
    epev.data.ptr = ptr;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, ptr->fd(), &epev);
    trace("del channel fd: %d, epfd: %d, add_ret", ptr->fd(), epfd_, ret);
    fatalif(ret < 0, "del channel fd: %d, epfd: %d, epctl_fail: %d, %s", ptr->fd(), epfd_, errno, strerror(errno));
}

void PollerEpoll::updateChannel(ChannelPtr ptr){
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));
    epev.events = ptr->events();
    epev.data.fd = ptr->fd();
    epev.data.ptr = ptr;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, ptr->fd(), &epev);
    trace("update channel fd: %d, epfd: %d, add_ret", ptr->fd(), epfd_, ret);
    fatalif(ret < 0, "update channel fd: %d, epfd: %d, epctl_fail: %d, %s", ptr->fd(), epfd_, errno, strerror(errno));
}

PollerPtr createPoller(){
    return new PollerEpoll();
}

} // hd
