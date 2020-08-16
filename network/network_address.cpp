//
// Created by root on 20-8-16.
//

#include "network_address.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>
#include "common/logging.h"

namespace network{
namespace address{

int setNonBlock(int fd, bool value) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return errno;
    }
    if (value) {
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

int setReuseAddr(int fd, bool value) {
    int flag = value;
    int len = sizeof flag;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
}

int setReusePort(int fd, bool value) {
#ifndef SO_REUSEPORT
    fatalif(value, "SO_REUSEPORT not supported");
return 0;
#else
    int flag = value;
    int len = sizeof flag;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &flag, len);
#endif
}

int setNoDelay(int fd, bool value) {
    int flag = value;
    int len = sizeof flag;
    return setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &flag, len);
}

using std::string;
using namespace comm::log;

Ip4Addr::Ip4Addr(const string &host, short port) {
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (host.size()) {
        addr_.sin_addr = port::getHostByName(host);
    } else {
        addr_.sin_addr.s_addr = INADDR_ANY;
    }
    if (addr_.sin_addr.s_addr == INADDR_NONE) {
        error("cannot resove %s to ip", host.c_str());
    }
}

std::string Ip4Addr::toString() const {
    uint32_t uip = addr_.sin_addr.s_addr;
    return comm::util::util::format("%d.%d.%d.%d:%d", (uip >> 0) & 0xff, (uip >> 8) & 0xff, (uip >> 16) & 0xff, (uip >> 24) & 0xff, ntohs(addr_.sin_port));
}

string Ip4Addr::ip() const {
    uint32_t uip = addr_.sin_addr.s_addr;
    return comm::util::util::format("%d.%d.%d.%d", (uip >> 0) & 0xff, (uip >> 8) & 0xff, (uip >> 16) & 0xff, (uip >> 24) & 0xff);
}

short Ip4Addr::port() const {
    return ntohs(addr_.sin_port);
}

unsigned int Ip4Addr::ipInt() const {
    return ntohl(addr_.sin_addr.s_addr);
}
bool Ip4Addr::isIpValid() const {
    return addr_.sin_addr.s_addr != INADDR_NONE;
}

} // address
} // network
