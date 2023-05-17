//
// Created by wenwen on 2023/5/5.
// hd_xxx代表的是handy类似的lib

#pragma once
#include "common/noncopyable.h"
#include "hd_channel.h"

namespace hd{

class Acceptor{
public:
    Acceptor();

private:
    ChannelPtr channel_;

};

} // hd
