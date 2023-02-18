//
// Created by wenwen on 2023/2/3.
// hd_xxx代表的是handy类似的lib

#pragma once

namespace hd{

class Channel;

using ChannelPtr = Channel*;

struct PollerBase{
    virtual ~PollerBase() = 0;

    virtual void addChannel(ChannelPtr) = 0;
    virtual void removeChannel(ChannelPtr) = 0;
    virtual void updateChannel(ChannelPtr) = 0;
};

} // hd
