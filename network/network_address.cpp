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

socklen_t Ip4Addr::socklen()const{
    return static_cast<socklen_t>(sizeof(struct sockaddr_in));
}

} // address
} // network
