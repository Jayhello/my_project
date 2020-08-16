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

struct Ip4Addr{
    Ip4Addr(const std::string &host, short port);
    Ip4Addr(short port = 0) : Ip4Addr("", port) {}
    Ip4Addr(const struct sockaddr_in &addr) : addr_(addr){};

    std::string toString() const;
    std::string ip() const;
    short port() const;
    unsigned int ipInt() const;
    // if you pass a hostname to constructor, then use this to check error
    bool isIpValid() const;
    struct sockaddr_in &getAddr() {
        return addr_;
    }
    static std::string hostToIp(const std::string &host) {
        Ip4Addr addr(host, 0);
        return addr.ip();
    }    
    
private:
    struct sockaddr_in addr_;    
};
    
} // address
} // network
