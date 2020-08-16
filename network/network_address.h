//
// Created by root on 20-8-16.
//

#pragma once

#include <netinet/in.h>
#include <string.h>
#include <algorithm>
#include <string>
#include "port_posix.h"

namespace network{
namespace address{

template <class T>
static T hton(T v) {
    return port::htobe(v);
}
template <class T>
static T ntoh(T v) {
    return port::htobe(v);
}
int setNonBlock(int fd, bool value = true);
int setReuseAddr(int fd, bool value = true);
int setReusePort(int fd, bool value = true);
int setNoDelay(int fd, bool value = true);

    
    
} // address
} // network
