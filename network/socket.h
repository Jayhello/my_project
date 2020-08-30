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

// if noncopyable and explicit then construct will be a problem
//class Socket:comm::util::noncopyable{
class Socket{
public:
    enum enSocketType{
        TCP = 1,
        UDP = 2,
    };

//    explicit Socket(int fd, int domain = AF_INET);
    Socket(int fd, int domain = AF_INET);
    Socket():fd_(-1), domain_(AF_INET){}

//    explicit Socket(const Socket&);
//
    Socket& operator=(const Socket& other){
        fd_ = other.fd_;
        domain_ = other.domain_;
    }
//
//    explicit Socket(Socket &&);

    static Socket CreateTcpSocket(){
        int domain = AF_INET;
        int fd = ::socket(domain, SOCK_STREAM, IPPROTO_TCP);
        return Socket(fd, domain);
    }

    static Socket CreateTcpFd(){
        int domain = AF_INET;
        int fd = ::socket(domain, SOCK_STREAM, IPPROTO_TCP);
        return fd;
    }

    static Socket CreateUdpSocket(){
        int domain = AF_INET;
        int fd = ::socket(domain, SOCK_DGRAM, IPPROTO_UDP);
        return Socket(fd, domain);
    }

    ~Socket();

    int GetFd()const {return fd_;}
    int GetDomain()const{return domain_;}

    int SetSocketOpt(int opt, const void* val, socklen_t opt_len, int level = SOL_SOCKET);

    int GetSocketOpt(int opt, void* val, socklen_t* opt_len, int level = SOL_SOCKET);

    int Bind(const address::Ip4Addr& addr);

    int Connect(const address::Ip4Addr& addr);

    int Listen(int backlog = SOMAXCONN);

    int Accept(address::Ip4Addr* p_addr);

    int IsClose()const{return fd_ < 0;}

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