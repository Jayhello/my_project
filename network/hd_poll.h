//
// Created by wenwen on 2023/2/3.
// hd_xxx代表的是handy类似的lib

#pragma once
#include "common/noncopyable.h"

namespace hd{

class Channel;

using ChannelPtr = Channel*;

struct PollerBase : public comm::noncopyable{
    virtual ~PollerBase() = 0;

    virtual void addChannel(ChannelPtr) = 0;
    virtual void removeChannel(ChannelPtr) = 0;
    virtual void updateChannel(ChannelPtr) = 0;
};

using PollerPtr = PollerBase*;

PollerPtr createPoller();

} // hd
