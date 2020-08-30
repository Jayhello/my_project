//
// Created by root on 20-8-16.
//

#include "socket.h"
#include <fcntl.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <unistd.h>
#include "common/logging.h"

using namespace comm::log;
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
//
//Socket::Socket(const Socket& other):fd_(other.fd_), domain_(other.domain_){
//
//}
//
//    Socket& Socket::operator=(const Socket& other){
//
//    }
//
//Socket::Socket(Socket && other){
//    fd_ = other.fd_;
//    domain_ = other.domain_;
//
//    other.fd_ = -1;
//}

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
    if(ret){
        error("bind err: %d, addr: %s, fd: %d", errno, addr.toString().c_str(), fd_);
    }
    assert(0 == ret);
}

int Socket::Connect(const address::Ip4Addr& addr){
    int ret = ::connect(fd_, addr.getAddr(), addr.socklen());
    assert(ret >= 0 and errno != EINPROGRESS);
}

int Socket::Listen(int backlog){
    int ret = ::listen(fd_, backlog);
    assert(ret >= 0);
}

int Socket::Accept(address::Ip4Addr* p_peer_addr){
    int new_fd = -1;

    struct sockaddr addr;
    socklen_t len = static_cast<socklen_t>(sizeof(addr));

    while ((new_fd = ::accept4(fd_, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC)) < 0 and  EINTR == errno){
        ;
    }

    if(nullptr != p_peer_addr){
        *p_peer_addr = address::Ip4Addr(&addr);
    }

    return new_fd;
}

int Socket::Close(){
    if(fd_ > 0){
        ::close(fd_);
        fd_ = -1;
    }
}

int Socket::Send(const void* buff, size_t len, int flag){
    return ::send(fd_, buff, len, flag);
}

int Socket::Recv(void* buff, size_t len, int flag){
    return ::recv(fd_, buff, len, flag);
}

int Socket::SendTo(const void* buff, size_t len, int flag, const address::Ip4Addr& addr){
    return ::sendto(fd_, buff, len, flag, addr.getAddr(), addr.socklen());
}

int Socket::RecvFrom(void* buff, size_t len, int flag, address::Ip4Addr* p_addr){
    struct sockaddr addr;
    socklen_t sock_len = static_cast<socklen_t>(sizeof(addr));

    int ret = ::recvfrom(fd_, buff, len, flag, &addr, &sock_len);
    if(ret > 0 and p_addr){
        *p_addr = address::Ip4Addr(&addr);
    }

    return ret;
}

void Socket::SetReuseAddr(){
    int flag = 1;

    int ret = SetSocketOpt(SO_REUSEADDR, (void*)(&flag), (socklen_t)(sizeof(flag)), SOL_SOCKET);
    assert(0 == ret);
}

void Socket::SetReusePort(){
    int flag = 1;

    int ret = SetSocketOpt(SO_REUSEPORT, (void*)(&flag), (socklen_t)(sizeof(flag)), SOL_SOCKET);
    assert(0 == ret);
}

void Socket::SetBlock(bool block){
    setNonBlock(fd_, true);
}

} // socket
} // network
