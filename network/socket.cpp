//
// Created by root on 20-8-16.
//

#include "socket.h"
#include <fcntl.h>
#include <netinet/tcp.h>
#include <assert.h>

namespace network{
namespace socket{

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

Socket::Socket(int fd, int domain):fd_(fd), domain_(domain){

}

Socket::~Socket(){
    Close();
}

int Socket::SetSocketOpt(int opt, const void* val, socklen_t opt_len, int level){
    return setsockopt(fd_, level, opt, &val, opt_len);;
}

int Socket::GetSocketOpt(int opt, void* val, socklen_t* opt_len, int level){
    return getsockopt(fd_, level, opt, val, opt_len);
}

int Socket::Bind(const address::Ip4Addr& addr){
    int ret = ::bind(fd_, addr.getAddr(), addr.socklen());
    assert(0 == ret);
}

int Socket::Connect(const address::Ip4Addr& addr){
    int ret = ::connect(fd_, addr.getAddr(), addr.socklen());
    assert(ret >= 0 and errno != EINPROGRESS);
}

int Socket::Listen(int backlog = SOMAXCONN){
    int ret = ::listen(fd_, backlog);
    assert(ret >= 0);
}

int Socket::Accept(address::Ip4Addr* p_addr){
    int ret = 0;
}

int Socket::Close(){

}

int Socket::Send(const void* buff, size_t len, int flag = 0){

}

int Socket::Recv(void* buff, size_t len, int flag = 0){

}

int Socket::SendTo(const void* buff, size_t len, int flag, const address::Ip4Addr& addr){

}

int Socket::RecvFrom(void* buff, size_t len, int flag, address::Ip4Addr* p_addr){

}

void Socket::SetReuseAddr(){

}

void Socket::SetReusePort(){

}

void Socket::SetBlock(bool block){

}

} // socket
} // network
