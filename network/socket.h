//
// Created by root on 20-8-16.
//
#pragma once

#include "network_address.h"
#include "common/commmon.h"

namespace network{
namespace socket{

int setNonBlock(int fd, bool value = true);
int setReuseAddr(int fd, bool value = true);
int setReusePort(int fd, bool value = true);
int setNoDelay(int fd, bool value = true);

class Socket:comm::util::noncopyable{
public:
    explicit Socket(int fd, int domain);

    ~Socket();

    int GetFd()const {return fd_;}
    int GetDomain()const{return domain_;}

    int SetSocketOpt(int opt, const void* val, socklen_t opt_len, int level = SOL_SOCKET);

    int GetSocketOpt(int opt, void* val, socklen_t* opt_len, int level = SOL_SOCKET);

    int Bind(const address::Ip4Addr& addr);

    int Connect(const address::Ip4Addr& addr);

    int Listen(int backlog = SOMAXCONN);

    int Accept(address::Ip4Addr* p_addr);

    int Close();

    int Send(const void* buff, size_t len, int flag = 0);

    int Recv(void* buff, size_t len, int flag = 0);

    int SendTo(const void* buff, size_t len, int flag, const address::Ip4Addr& addr);

    int RecvFrom(void* buff, size_t len, int flag, address::Ip4Addr* p_addr);
    
    void SetReuseAddr();

    void SetReusePort();
    
    void SetBlock(bool block = false);
    
private:
    int fd_;
    int domain_;
};

} // socket
} // network