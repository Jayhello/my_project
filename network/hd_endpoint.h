//
// Created by Administrator on 2023/5/18.
//

#pragma once
#include "common/util.h"

namespace hd{


using namespace comm;

struct EndPoint{
    std::string sip;
    int port;



    inline std::string toString()const{
        return comm::util::util::format("ip:%s,port:%d", sip.c_str(), port);
    }

};


} // hd
