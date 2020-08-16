//
// Created by root on 20-8-16.
//

#include "network_address.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>

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
    
} // address
} // network
